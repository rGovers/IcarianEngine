// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/BlockAllocator.h"

#include <limits>

#include "Core/DataTypes/ThreadGuard.h"
#include "Core/IcarianDefer.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    BlockAllocator::BlockAllocator(uint32_t a_blockSize, Allocator* a_upstreamAllocator)
    {
        ICARIAN_ASSERT_R(a_blockSize > OverHeadSize);

        m_upstreamAllocator = a_upstreamAllocator;

        m_blockSize = a_blockSize;
        m_block = AllocateBlock();
    }
    BlockAllocator::~BlockAllocator()
    {
        BlockHeader* block = m_block;
        while (block != NULL)
        {
            BlockHeader* next = block->Next;
            IDEFER(block = next);

            VerifyBlock(block);

            ICARIAN_ASSERT_MSG_R(IsBlockFree(block), "Block leaked memory");

            FreeBlock(block);
        }
    }

    void BlockAllocator::SetHeaderOffsets(AllocationHeader* a_prevHeader, AllocationHeader* a_nextHeader)
    {
        if (a_prevHeader != NULL && a_nextHeader != NULL)
        {
            ICARIAN_ASSERT(a_nextHeader > a_prevHeader);
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
    void BlockAllocator::SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;
#endif
    }
    void BlockAllocator::ClearCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = 0;
        a_header->CanaryB = 0;
#endif
    }

    void BlockAllocator::VerifyBlock(const BlockHeader* a_header)
    {
#ifdef DEBUG
        ICARIAN_ASSERT(a_header->CanaryA == CanaryValue);
        ICARIAN_ASSERT(a_header->CanaryB == CanaryValue);

        const AllocationHeader* allocHeader = GetFirstAllocation(a_header);
        while (allocHeader != NULL)
        {
            IDEFER(allocHeader = NextAllocation(allocHeader));

            ICARIAN_ASSERT(VerifyAllocation(allocHeader));
        }
#endif
    }

    BlockAllocator::BlockHeader* BlockAllocator::AllocateBlock()
    {
        void* ptr = m_upstreamAllocator->ZAllocate((uint64_t)m_blockSize, alignof(BlockHeader));

        BlockHeader* header = (BlockHeader*)ptr;
#ifdef DEBUG
        header->CanaryA = CanaryValue;
        header->CanaryB = CanaryValue;
#endif

        void* allocPtr = AlignTo((uint8_t*)ptr + sizeof(BlockHeader), alignof(AllocationHeader));
        AllocationHeader* allocHeader = (AllocationHeader*)allocPtr;
        SetCanary(allocHeader);
        ISETBIT(allocHeader->Flags, AllocationHeader::FreeFlagBit);
        allocHeader->BlockOffset = (uint32_t)((uintptr_t)allocPtr - (uintptr_t)header);

        header->Free = allocHeader;

        return header;
    }
    void BlockAllocator::FreeBlock(BlockHeader* a_block)
    {
        m_upstreamAllocator->Free(a_block);
    }

    void* BlockAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        ICARIAN_ASSERT(a_size < std::numeric_limits<uint32_t>::max());

        // Find an alignment that fits both the header and the allocation alignment
        const uint32_t targetAlignment = ILAMBDA(
        {
            if (a_alignment > alignof(AllocationHeader))
            {
                ILRETURN AlignTo(a_alignment, alignof(AllocationHeader));
            }

            ILRETURN (uint32_t)alignof(AllocationHeader);
        });

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
                ICARIAN_ASSERT(allocationHeader != nextAllocation);
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
                    const uintptr_t paddedHeaderEnd = AlignTo((uintptr_t)allocationHeader + sizeof(AllocationHeader), (uintptr_t)targetAlignment);
                    const uintptr_t paddedAllocationEnd = AlignTo(paddedHeaderEnd + a_size, alignof(AllocationHeader));
                    const uintptr_t paddedSizeEnd = paddedAllocationEnd + sizeof(AllocationHeader);

                    const uint64_t usedSize = (uint64_t)(paddedSizeEnd - (uintptr_t)blockHeader);
                    if (usedSize >= m_blockSize)
                    {
                        break;
                    }

                    // Need to realign the header so that it respects the requested alignment
                    // Always align forwards never backwards as we may overwrite existing memory otherwise
                    void* ptr = (void*)paddedHeaderEnd;
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

                    AllocationHeader* newAllocation = (AllocationHeader*)paddedAllocationEnd;
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

                void* ptr = AlignTo((uint8_t*)allocationHeader + sizeof(AllocationHeader), (uintptr_t)targetAlignment);

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
                        AllocationHeader* iter = NextAllocation(allocationHeader);
                        while (iter != NULL)
                        {
                            if (IISBITSET(iter->Flags, AllocationHeader::FreeFlagBit))
                            {
                                blockHeader->Free = iter;

                                break;
                            }

                            iter = NextAllocation(iter);
                        }
                    }

                    return ptr;
                }

                AllocationHeader* newHeader = (AllocationHeader*)AlignTo((uint8_t*)ptr + a_size, alignof(AllocationHeader));
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
    void BlockAllocator::Free(void* a_ptr)
    {
        if (a_ptr == nullptr)
        {
            return;
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        ICARIAN_ASSERT(VerifyAllocation(header));

        BlockHeader* blockHeader = (BlockHeader*)((uint8_t*)header - header->BlockOffset);
        const ThreadGuard g = ThreadGuard(blockHeader->Lock);

        AllocationHeader* nextAllocation = NextAllocation(header);
        if (nextAllocation != NULL)
        {
            ICARIAN_ASSERT(VerifyAllocation(nextAllocation));
        }
        AllocationHeader* prevAllocation = PrevAllocation(header);
        if (prevAllocation != NULL)
        {
            ICARIAN_ASSERT(VerifyAllocation(prevAllocation));
        }

        ICARIAN_ASSERT(!IISBITSET(header->Flags, AllocationHeader::FreeFlagBit));

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
                ICARIAN_ASSERT(VerifyAllocation(nextNeighbour));
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
    void* BlockAllocator::Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        // Seems like a reasonable limit for a single allocation and would be a bug if we hit this
        ICARIAN_ASSERT(a_size < std::numeric_limits<uint32_t>::max());

        void* headerPtr = (uint8_t*)a_ptr - sizeof(AllocationHeader);
        AllocationHeader* header = (AllocationHeader*)headerPtr;
        ICARIAN_ASSERT(VerifyAllocation(header));

        // If the allocation is valid there should be another header after the allocation
        ICARIAN_ASSERT(header->NextOffset >= sizeof(AllocationHeader));
        ICARIAN_ASSERT(!IISBITSET(header->Flags, AllocationHeader::FreeFlagBit));

        const uint64_t size = (uint64_t)header->NextOffset - sizeof(AllocationHeader);
        if (size >= a_size && IsAligned((uint8_t*)a_ptr, (uintptr_t)a_alignment))
        {
            // Still have room so just return the same pointer
            return a_ptr;
        }

        IDEFER(Free(a_ptr));

        void* nextPtr = Allocate(a_size, a_alignment);
        memcpy(nextPtr, a_ptr, size);

        return nextPtr;
    }

    void BlockAllocator::TrimBlocks()
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
}

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
