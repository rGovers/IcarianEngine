// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocators/ComplexAllocator.h"

#ifdef __linux__
#if defined (__GNUC__) && !defined (__clang__)
#include <cxxabi.h>
#endif
#endif

#include <cstring>
#include <limits>

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "DataTypes/SpinLock.h"
#include "DataTypes/ThreadGuard.h"
#include "IcarianError.h"
#include "IcarianMemory.h"
#include "Trace.h"

ICARIAN_PUSH_FASTALLOCTOR

// First time implementing an allocator that is not just a ring so please forgive me for the mess
// Should probably have looked at existing allocators but fuck it we wing it in this house
class BlockAllocator : public ComplexAllocator
{
private:
    struct AllocationHeader
    {
        constexpr static uint32_t FreeFlagBit = 0;

#ifdef DEBUG
        uintptr_t CanaryA;
#endif
        uint32_t Flags;
        uint32_t BlockOffset;
        uint32_t PrevOffset;
        uint32_t NextOffset;
#ifdef DEBUG
        uintptr_t CanaryB;
#endif
    };

    struct BlockHeader
    {
#ifdef DEBUG
        uintptr_t CanaryA;
#endif
        BlockHeader* Next;
        AllocationHeader* Free;
        AllocationHeader* First;
        SpinLock Lock;
#ifdef DEBUG
        uintptr_t CanaryB;
#endif
    };

    // Annoying that I have to use a 16 byte alignment but who am I to argue with Intel and the C++ standard
    constexpr static uint32_t OverHeadSize = sizeof(BlockHeader) + sizeof(AllocationHeader) * 2 + BaseAlignment;
    constexpr static uint32_t BacktraceSize = 16;
    constexpr static uint64_t CanaryValue = 0xCCCCCCCCCCCCCCCC;

    Allocator*   m_upstreamAllocator;

    BlockHeader* m_block;
    uint32_t     m_blockSize;

    static AllocationHeader* AllocationFromPointer(const void* a_ptr)
    {
        return (AllocationHeader*)((uint8_t*)a_ptr - sizeof(AllocationHeader));
    }

    static AllocationHeader* GetFirstAllocation(const BlockHeader* a_header)
    {
        if (a_header->First != NULL)
        {
            return a_header->First;
        }

        const void* allocPtr = AlignTo((uint8_t*)a_header + sizeof(BlockHeader), BaseAlignment);

        return (AllocationHeader*)allocPtr;
    }

    static AllocationHeader* NextAllocation(const AllocationHeader* a_header)
    {
        if (a_header->NextOffset == 0)
        {
            return NULL;
        }

        return (AllocationHeader*)((uint8_t*)a_header + a_header->NextOffset);
    }
    static AllocationHeader* PrevAllocation(const AllocationHeader* a_header)
    {
        if (a_header->PrevOffset == 0)
        {
            return NULL;
        }

        return (AllocationHeader*)((uint8_t*)a_header - a_header->PrevOffset);
    }

    static bool VerifyAllocation(const AllocationHeader* a_header)
    {
#ifdef DEBUG
        if (a_header->CanaryA != CanaryValue || a_header->CanaryB != CanaryValue)
        {
            return false;
        }
#endif

        return true;
    }

    static void SetHeaderOffsets(AllocationHeader* a_prevHeader, AllocationHeader* a_nextHeader)
    {
        if (a_prevHeader != NULL && a_nextHeader != NULL)
        {
            IVERIFY(a_nextHeader > a_prevHeader);
        }

        const uint32_t offset = (uint32_t)((uintptr_t)a_nextHeader - (uintptr_t)a_prevHeader);

        if (a_prevHeader != NULL)
        {
            a_prevHeader->NextOffset = 0;

            if (a_nextHeader != NULL)
            {
                a_prevHeader->NextOffset = offset;
            }
        }

        if (a_nextHeader != NULL)
        {
            a_nextHeader->PrevOffset = 0;

            if (a_prevHeader != NULL)
            {
                a_nextHeader->PrevOffset = offset;
            }
        }
    }
    static void SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;
#endif
    }
    static void ClearCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = 0;
        a_header->CanaryB = 0;
#endif
    }

    static void VerifyBlock(const BlockHeader* a_header)
    {
#ifdef DEBUG
        if (a_header->CanaryA != CanaryValue || a_header->CanaryB != CanaryValue)
        {
            IERROR("Corrupted block header");
        }

        const AllocationHeader* allocHeader = GetFirstAllocation(a_header);

        while (allocHeader != NULL)
        {
            IDEFER(allocHeader = NextAllocation(allocHeader));

            IVERIFY(VerifyAllocation(allocHeader));
        }
#endif
    }

    static bool IsBlockFree(const BlockHeader* a_header)
    {
        const AllocationHeader* allocHeader = GetFirstAllocation(a_header);
        while (allocHeader != NULL)
        {
            IDEFER(allocHeader = NextAllocation(allocHeader));

            if (!IISBITSET(allocHeader->Flags, AllocationHeader::FreeFlagBit))
            {
                return false;
            }
        }

        return true;
    }

    BlockHeader* AllocateBlock()
    {
        void* ptr = m_upstreamAllocator->Allocate((uint64_t)m_blockSize, BaseAlignment);
        memset(ptr, 0, (size_t)m_blockSize);

        BlockHeader* header = (BlockHeader*)ptr;
#ifdef DEBUG
        header->CanaryA = CanaryValue;
        header->CanaryB = CanaryValue;
#endif

        void* allocPtr = AlignTo((uint8_t*)ptr + sizeof(BlockHeader), BaseAlignment);
        AllocationHeader* allocHeader = (AllocationHeader*)allocPtr;
        SetCanary(allocHeader);
        ISETBIT(allocHeader->Flags, AllocationHeader::FreeFlagBit);
        allocHeader->BlockOffset = (uint32_t)((uintptr_t)allocPtr - (uintptr_t)header);

        header->Free = allocHeader;

        return header;
    }

    void FreeBlock(BlockHeader* a_block)
    {
        m_upstreamAllocator->Free(a_block);
    }

protected:

public:
    BlockAllocator(uint32_t a_blockSize, Allocator* a_upstreamAllocator)
    {
        if (a_blockSize < OverHeadSize)
        {
            IERROR("Block too small");
        }

        m_upstreamAllocator = a_upstreamAllocator;

        m_blockSize = a_blockSize;
        m_block = AllocateBlock();
    }
    virtual ~BlockAllocator()
    {
        BlockHeader* block = m_block;
        while (block != NULL)
        {
            BlockHeader* next = block->Next;
            IDEFER(block = next);

            VerifyBlock(block);

            if (!IsBlockFree(block))
            {
                IERROR("Block allocator leaked memory");
            }

            FreeBlock(block);
        }
    }

    inline Allocator* GetUpstreamAllocator() const
    {
        return m_upstreamAllocator;
    }

    [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size == 0)
        {
            return nullptr;
        }

        // Seems like a reasonable limit for a single allocation and would be a bug if we hit this
        if (a_size >= std::numeric_limits<uint32_t>::max())
        {
            IERROR("Allocation too large to serve");
        }

        BlockHeader* blockHeader = m_block;
        while (true)
        {
            const ThreadGuard g = ThreadGuard(blockHeader->Lock);

            VerifyBlock(blockHeader);

            AllocationHeader* firstHeader = GetFirstAllocation(blockHeader);
            AllocationHeader* allocationHeader = blockHeader->Free;
            if (allocationHeader == NULL)
            {
                allocationHeader = firstHeader;
            }

            AllocationHeader* prevAllocation = PrevAllocation(allocationHeader);
            while (allocationHeader != NULL)
            {
                AllocationHeader* nextAllocation = NextAllocation(allocationHeader);
                IVERIFY(allocationHeader != nextAllocation);
                IDEFER(
                {
                    prevAllocation = allocationHeader;
                    allocationHeader = nextAllocation;
                });

                if (!IISBITSET(allocationHeader->Flags, AllocationHeader::FreeFlagBit))
                {
                    continue;
                }

                if (nextAllocation == NULL)
                {
                    // If next is NULL we are at the end so move onto another block
                    const uint64_t usedSize = ((uintptr_t)allocationHeader + (uint64_t)(sizeof(AllocationHeader) * 2) + a_size) - (uintptr_t)blockHeader;
                    if (usedSize + a_alignment * 2 >= m_blockSize)
                    {
                        break;
                    }

                    // Need to realign the header so that it respects the requested alignment
                    // Always align forwards never backwards as we may overwrite existing memory otherwise
                    void* ptr = AlignTo((uint8_t*)allocationHeader + sizeof(AllocationHeader), (uintptr_t)a_alignment);
                    AllocationHeader* correctedHeader = AllocationFromPointer(ptr);
                    IDEFER(SetCanary(correctedHeader));

                    ClearCanary(allocationHeader);

                    SetHeaderOffsets(prevAllocation, correctedHeader);

                    correctedHeader->BlockOffset = (uint32_t)((uintptr_t)correctedHeader - (uintptr_t)blockHeader);
                    correctedHeader->Flags = 0;

                    if (firstHeader == allocationHeader)
                    {
                        blockHeader->First = correctedHeader;
                    }

                    AllocationHeader* newAllocation = (AllocationHeader*)AlignTo((uint8_t*)ptr + a_size, BaseAlignment);
                    IDEFER(SetCanary(newAllocation));

                    SetHeaderOffsets(correctedHeader, newAllocation);
                    SetHeaderOffsets(newAllocation, NULL);

                    newAllocation->BlockOffset = (uint32_t)((uintptr_t)newAllocation - (uintptr_t)blockHeader);
                    newAllocation->Flags = 0;
                    ISETBIT(newAllocation->Flags, AllocationHeader::FreeFlagBit);

                    if (blockHeader->Free == allocationHeader)
                    {
                        blockHeader->Free = newAllocation;
                    }

                    return ptr;
                }

                void* ptr = AlignTo((uint8_t*)allocationHeader + sizeof(AllocationHeader), (uintptr_t)a_alignment);

                const uintptr_t size = (uintptr_t)nextAllocation - (uintptr_t)ptr;
                if (size < a_size)
                {
                    continue;
                }

                AllocationHeader* correctedHeader = AllocationFromPointer(ptr);
                IDEFER(SetCanary(correctedHeader));

                ClearCanary(allocationHeader);

                SetHeaderOffsets(prevAllocation, correctedHeader);
                SetHeaderOffsets(correctedHeader, nextAllocation);

                correctedHeader->BlockOffset = (uint32_t)((uintptr_t)correctedHeader - (uintptr_t)blockHeader);
                correctedHeader->Flags = 0;

                if (firstHeader == allocationHeader)
                {
                    blockHeader->First = correctedHeader;
                }

                const uint64_t remainingSize = size - a_size;
                const bool roomToSlice = remainingSize > 128;
                if (!roomToSlice)
                {
                    if (blockHeader->Free == allocationHeader)
                    {
                        AllocationHeader* nextFree = NULL;
                        AllocationHeader* iter = NextAllocation(allocationHeader);
                        while (iter != NULL)
                        {
                            if (IISBITSET(iter->Flags, AllocationHeader::FreeFlagBit))
                            {
                                nextFree = iter;

                                break;
                            }

                            iter = NextAllocation(iter);
                        }

                        blockHeader->Free = nextFree;
                    }

                    return ptr;
                }

                AllocationHeader* newHeader = (AllocationHeader*)AlignTo((uint8_t*)ptr + a_size, BaseAlignment);
                IDEFER(SetCanary(newHeader));

                SetHeaderOffsets(correctedHeader, newHeader);
                SetHeaderOffsets(newHeader, nextAllocation);

                newHeader->BlockOffset = (uint32_t)((uintptr_t)newHeader - (uintptr_t)blockHeader);
                newHeader->Flags = 0;
                ISETBIT(newHeader->Flags, AllocationHeader::FreeFlagBit);

                if (blockHeader->Free == allocationHeader)
                {
                    blockHeader->Free = newHeader;
                }

                return ptr;
            }

            if (blockHeader->Next == NULL)
            {
                blockHeader->Next = AllocateBlock();
            }

            blockHeader = blockHeader->Next;
        }
    }

    virtual void Free(void* a_ptr)
    {
        if (a_ptr == nullptr)
        {
            return;
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        IVERIFY(VerifyAllocation(header));

        BlockHeader* blockHeader = (BlockHeader*)((uint8_t*)header - header->BlockOffset);
        const ThreadGuard g = ThreadGuard(blockHeader->Lock);

        AllocationHeader* nextAllocation = NextAllocation(header);
        if (nextAllocation != NULL)
        {
            IVERIFY(VerifyAllocation(nextAllocation));
        }
        AllocationHeader* prevAllocation = PrevAllocation(header);
        if (prevAllocation != NULL)
        {
            IVERIFY(VerifyAllocation(prevAllocation));
        }

        IVERIFY(!IISBITSET(header->Flags, AllocationHeader::FreeFlagBit));

        AllocationHeader* firstHeader = GetFirstAllocation(blockHeader);
        if (firstHeader == header)
        {
            blockHeader->First = NULL;

            AllocationHeader* offHead = GetFirstAllocation(blockHeader);
            IDEFER(header = offHead);
            IDEFER(SetCanary(offHead));

            ClearCanary(header);

            SetHeaderOffsets(NULL, offHead);
            SetHeaderOffsets(offHead, nextAllocation);

            offHead->Flags = 0;
            offHead->BlockOffset = (uint32_t)((uintptr_t)offHead - (uintptr_t)blockHeader);
        }

        ISETBIT(header->Flags, AllocationHeader::FreeFlagBit);

        AllocationHeader* nextNeighbour = NULL;
        if (nextAllocation != NULL && IISBITSET(nextAllocation->Flags, AllocationHeader::FreeFlagBit))
        {
            ClearCanary(nextAllocation);

            nextNeighbour = NextAllocation(nextAllocation);
            if (nextNeighbour != NULL)
            {
                IVERIFY(VerifyAllocation(nextNeighbour));
            }

            SetHeaderOffsets(header, nextNeighbour);
        }

        if (prevAllocation != NULL && IISBITSET(prevAllocation->Flags, AllocationHeader::FreeFlagBit))
        {
            ClearCanary(header);

            if (nextNeighbour != NULL)
            {
                SetHeaderOffsets(prevAllocation, nextNeighbour);
            }
            else if (nextAllocation != NULL && !IISBITSET(nextAllocation->Flags, AllocationHeader::FreeFlagBit))
            {
                SetHeaderOffsets(prevAllocation, nextAllocation);
            }
            else
            {
                SetHeaderOffsets(prevAllocation, NULL);
            }

            return;
        }

        // I was being dumb if the last check succeeded it means this check will fail as there was a prior block free
        // Should be good now that the last check returns
        if (header < blockHeader->Free)
        {
            blockHeader->Free = header;
        }
    }

    // Have the data to implement realloc so just do it
    // NOTE: This is not thread safe
    [[nodiscard]] virtual void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        // Seems like a reasonable limit for a single allocation and would be a bug if we hit this
        if (a_size >= std::numeric_limits<uint32_t>::max())
        {
            IERROR("Allocation too large to serve");
        }

        void* headerPtr = (uint8_t*)a_ptr - sizeof(AllocationHeader);
        AllocationHeader* header = (AllocationHeader*)headerPtr;
        IVERIFY(VerifyAllocation(header));

        // If the allocation is valid there should be another header after the allocation
        IVERIFY(header->NextOffset >= sizeof(AllocationHeader));
        IVERIFY(!IISBITSET(header->Flags, AllocationHeader::FreeFlagBit));

        const uint64_t size = (uint64_t)header->NextOffset - sizeof(AllocationHeader);
        if (size >= a_size)
        {
            // Still have room so just return the same pointer
            return a_ptr;
        }

        IDEFER(Free(a_ptr));

        void* nextPtr = Allocate(a_size, a_alignment);
        memcpy(nextPtr, a_ptr, size);

        return nextPtr;
    }

    void TrimBlocks()
    {
        BlockHeader* prev = m_block;
        BlockHeader* block = prev->Next;
        while (block != NULL)
        {
            block->Lock.Lock();

            BlockHeader* next = block->Next;

            if (IsBlockFree(block))
            {
                const ThreadGuard g = ThreadGuard(prev->Lock);

                prev->Next = next;

                block->Lock.Unlock();

                FreeBlock(block);

                // I cannot be fucked fixing deallocating multiple and can just spread it across several calls anyway so not fussed
                break;
            }

            block->Lock.Unlock();

            prev = block;
            block = next;
        }
    }

    inline uint64_t GetBlockSize() const
    {
        return m_blockSize;
    }
};

ICARIAN_POP_FASTALLOCTOR

// MIT License
// 
// Copyright (c) 2026 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.