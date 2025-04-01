// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocator.h"

#ifdef __linux__
#if defined (__GNUC__) && !defined (__clang__)
#include <cxxabi.h>
#endif
#endif

#include <cstring>

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "DataTypes/SpinLock.h"
#include "DataTypes/ThreadGuard.h"
#include "IcarianError.h"
#include "Trace.h"

struct AllocationHeader;
struct BlockHeader;

struct AllocationHeader
{
    constexpr static uint32_t FreeFlagBit = 0;

#ifdef DEBUG
    uintptr_t CanaryA;
    char** Backtrace;
    uint32_t BacktraceSize;
#endif
    BlockHeader* Block;
    uint16_t NextOffset;
    uint16_t PrevOffset;
    uint32_t Flags;
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
    SpinLock Lock;
#ifdef DEBUG
    uintptr_t CanaryB;
#endif
};

// First time implementing an allocator that is not just a ring so please forgive me for the mess
// Should probably have looked at existing allocators but fuck it we wing it in this house
class BlockAllocator : public Allocator
{
private:
    // Annoying that I have to use a 16 byte alignment but who am I to argue with Intel and the C++ standard
    constexpr static uint32_t OverHeadSize = sizeof(BlockHeader) + sizeof(AllocationHeader) * 2 + BaseAlignment;
    constexpr static uint32_t BacktraceSize = 16;
    constexpr static uint64_t CanaryValue = 0xCCCCCCCCCCCCCCCC;

    BlockHeader* m_block;
    uint16_t     m_blockSize;

    char** CreateBacktrace(uint32_t* a_size)
    {
        *a_size = 0;

#ifdef __linux__
        void* array[BacktraceSize];
        *a_size = (uint32_t)backtrace(array, BacktraceSize);

        return backtrace_symbols(array, *a_size);
#endif

        return nullptr;
    }

    static AllocationHeader* GetFirstAllocation(const BlockHeader* a_header)
    {
        const void* allocPtr = Align((char*)a_header + sizeof(BlockHeader), BaseAlignment);

        return (AllocationHeader*)allocPtr;
    }

    static AllocationHeader* NextAllocation(const AllocationHeader* a_header)
    {
        if (a_header->NextOffset == 0)
        {
            return NULL;
        }

        return (AllocationHeader*)((char*)a_header + a_header->NextOffset);
    }
    static AllocationHeader* PrevAllocation(const AllocationHeader* a_header)
    {
        if (a_header->PrevOffset == 0)
        {
            return NULL;
        }

        return (AllocationHeader*)((char*)a_header - a_header->PrevOffset);
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

    static void SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;

#ifdef __linux__
        // I have the Kernel I may aswell use it against myself
        mprotect(&a_header->CanaryA, sizeof(a_header->CanaryA), PROT_READ);
        mprotect(&a_header->CanaryB, sizeof(a_header->CanaryB), PROT_READ);
#endif
#endif
    }
    static void ClearCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
#ifdef __linux__
        mprotect(&a_header->CanaryA, sizeof(a_header->CanaryA), PROT_READ | PROT_WRITE);
        mprotect(&a_header->CanaryB, sizeof(a_header->CanaryB), PROT_READ | PROT_WRITE);
#endif

        a_header->CanaryA = 0;
        a_header->CanaryB = 0;
#endif
    }

    static void PrintAllocationBacktrace(const AllocationHeader* a_header)
    {
#ifdef DEBUG
#ifdef __linux__
        for (uint32_t i = 0; i < a_header->BacktraceSize; ++i)
        {
// #if 1
#if defined (__GNUC__) && !defined (__clang__)
            char buffer[256];

            bool write = false;

            char* ptr = a_header->Backtrace[i];
            uint32_t writeIndex = 0;
            while (*ptr != 0)
            {
                const char chr = *ptr;

                if (chr == ')' || chr == '+')
                {
                    buffer[writeIndex] = 0;

                    break;
                }

                if (write)
                {
                    buffer[writeIndex++] = *ptr;
                }

                if (chr == '(')
                {
                    write = true;
                }

                ++ptr;
            }

            char* name = abi::__cxa_demangle(buffer, NULL, NULL, NULL);
            IDEFER(free(name));
#else
            char* name = a_header->Backtrace[i];
#endif
            if (name == nullptr)
            {
                continue;
            }

            printf("[%d] %s \n", i, name);
        }
#endif
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
        const AllocationHeader* lastHeader = NULL;

        while (allocHeader != NULL)
        {
            IDEFER(allocHeader = NextAllocation(allocHeader));

            if (!VerifyAllocation(allocHeader))
            {
                printf(" --------------------------------------- \n\n");
                printf("    Previous Allocation Callstack \n\n");
                printf(" --------------------------------------- \n");

                PrintAllocationBacktrace(lastHeader);

                IERROR("Corrupted allocation");
            }

            lastHeader = allocHeader;
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

    static void PrintLeaks(const BlockHeader* a_header)
    {
#ifdef DEBUG
#ifdef __linux__
        const AllocationHeader* allocHeader = GetFirstAllocation(a_header);
        while (allocHeader != NULL) 
        {
            IDEFER(allocHeader = NextAllocation(allocHeader));

            if (!IISBITSET(allocHeader->Flags, AllocationHeader::FreeFlagBit))
            {
                printf(" --------------------------------------- \n\n");
                printf("    Block Leaked Allocation Callstack \n\n");
                printf(" --------------------------------------- \n");

                PrintAllocationBacktrace(allocHeader);
            }
        }
#endif
#endif
    }

    BlockHeader* AllocateBlock()
    {
        TRACE("Allocating Block");

        void* ptr = MapMemory(m_blockSize);
        memset(ptr, 0, (size_t)m_blockSize);
        
        BlockHeader* header = (BlockHeader*)ptr;
#ifdef DEBUG
        header->CanaryA = CanaryValue;
        header->CanaryB = CanaryValue;
#endif

        void* allocPtr = Align((char*)ptr + sizeof(BlockHeader), BaseAlignment);
        AllocationHeader* allocHeader = (AllocationHeader*)allocPtr;
        SetCanary(allocHeader);
        ISETBIT(allocHeader->Flags, AllocationHeader::FreeFlagBit);
        allocHeader->Block = header;

        header->Free = allocHeader;

        TRACE("Allocated Block");

        return header;
    }

    void FreeBlock(BlockHeader* a_block)
    {
        TRACE("Freeing Block");

        UnmapMemory(a_block, m_blockSize);
    }
protected:

public:
    BlockAllocator(uint16_t a_blockSize)
    {
        if (a_blockSize < OverHeadSize)
        {
            IERROR("Block too small");
        }

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
                PrintLeaks(block);

                IERROR("Block allocator leaked memory");
            }

            FreeBlock(block);
        }
    }

    virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size == 0)
        {
            return nullptr;
        }

        IVERIFY(a_size < m_blockSize - OverHeadSize);

        BlockHeader* blockHeader = m_block;
        while (true)
        {
            const ThreadGuard g = ThreadGuard(blockHeader->Lock);

            VerifyBlock(blockHeader);

            AllocationHeader* allocationHeader = blockHeader->Free;
            if (allocationHeader == NULL)
            {
                allocationHeader = GetFirstAllocation(blockHeader);
            }

            while (allocationHeader != NULL)
            {
                AllocationHeader* nextAllocation = NextAllocation(allocationHeader);
                IVERIFY(allocationHeader != nextAllocation);
                IDEFER(allocationHeader = nextAllocation);

                if (!IISBITSET(allocationHeader->Flags, AllocationHeader::FreeFlagBit))
                {
                    continue;
                }

                if (nextAllocation == NULL)
                {
                    // If next is NULL we are at the end so move onto another block
                    const uint64_t usedSize = (uint64_t)(((char*)allocationHeader + sizeof(AllocationHeader) * 2 + a_size) - (char*)blockHeader);
                    if (usedSize + a_alignment > m_blockSize)
                    {
                        break;
                    }

                    void* ptr = (char*)allocationHeader + sizeof(AllocationHeader);

                    void* nextAllocationPtr = Align((char*)ptr + a_size, a_alignment);
                    AllocationHeader* nextAllocation = (AllocationHeader*)nextAllocationPtr;

                    const uint16_t offset = (uint16_t)(uintptr_t)((char*)nextAllocation - (char*)allocationHeader);

                    nextAllocation->PrevOffset = offset;
                    nextAllocation->NextOffset = 0;
                    nextAllocation->Block = blockHeader;
                    nextAllocation->Flags = 0;
                    ISETBIT(nextAllocation->Flags, AllocationHeader::FreeFlagBit);
                    SetCanary(nextAllocation);
#ifdef DEBUG
                    allocationHeader->Backtrace = CreateBacktrace(&allocationHeader->BacktraceSize);
#endif
                    allocationHeader->NextOffset = offset;

                    ICLEARBIT(allocationHeader->Flags, AllocationHeader::FreeFlagBit);

                    if (blockHeader->Free == allocationHeader)
                    {
                        blockHeader->Free = nextAllocation;
                    }

                    return ptr;
                }
                
                void* ptr = (char*)allocationHeader + sizeof(AllocationHeader);

                const uint64_t size = (uint64_t)((char*)nextAllocation - (char*)ptr);
                if (size < a_size)
                {
                    continue;
                }

                ICLEARBIT(allocationHeader->Flags, AllocationHeader::FreeFlagBit);
#ifdef DEBUG
                allocationHeader->Backtrace = CreateBacktrace(&allocationHeader->BacktraceSize);
#endif

                const uint64_t remainingSize = size - a_size;
                const bool roomToSlice = remainingSize > sizeof(AllocationHeader) * 3;
                if (!roomToSlice)
                {
                    // Not sure if I should just do the next header as even if it is not free the code should be able to handle it
                    // Doing this for correctness as I do not want to walk the list further as I already have a pointer "allocated"
                    // May walk the list or just set to the next in future need to think about it
                    if (blockHeader->Free == allocationHeader)
                    {
                        blockHeader->Free = NULL;
                    }

                    return ptr;
                }

                void* newHeaderPtr = Align((char*)ptr + a_size, a_alignment);
                AllocationHeader* newHeader = (AllocationHeader*)newHeaderPtr;
                
                const uint16_t nextOffset = (uint16_t)(uintptr_t)((char*)nextAllocation - (char*)newHeader);
                const uint16_t offset = (uint16_t)(uintptr_t)((char*)newHeader - (char*)allocationHeader);

                newHeader->PrevOffset = offset;
                newHeader->NextOffset = nextOffset;
                newHeader->Block = blockHeader;
                newHeader->Flags = 0;
                ISETBIT(newHeader->Flags, AllocationHeader::FreeFlagBit);
                SetCanary(newHeader);

                nextAllocation->PrevOffset = nextOffset;
                allocationHeader->NextOffset = offset;

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
        const bool isNull = (uintptr_t)a_ptr < sizeof(AllocationHeader);
        if (isNull)
        {
            return;
        }

        void* headerPtr = (char*)a_ptr - sizeof(AllocationHeader);
        AllocationHeader* header = (AllocationHeader*)headerPtr;
        IVERIFY(VerifyAllocation(header));

        BlockHeader* blockHeader = header->Block;
        const ThreadGuard g = ThreadGuard(blockHeader->Lock);

#ifdef DEBUG
#ifdef __linux__
        if (header->Backtrace != NULL)
        {
            free(header->Backtrace);
        }
#endif
#endif

        IVERIFY(!IISBITSET(header->Flags, AllocationHeader::FreeFlagBit));
        ISETBIT(header->Flags, AllocationHeader::FreeFlagBit);

        AllocationHeader* nextAllocation = NextAllocation(header);
        AllocationHeader* nextNeighbour = NULL;
        if (nextAllocation != NULL && IISBITSET(nextAllocation->Flags, AllocationHeader::FreeFlagBit))
        {
            IVERIFY(VerifyAllocation(nextAllocation));
            ClearCanary(nextAllocation);

            if (nextAllocation->NextOffset == 0)
            {
                header->NextOffset = 0;
            }
            else
            {
                nextNeighbour = NextAllocation(nextAllocation);

                header->NextOffset += nextAllocation->NextOffset;
                nextNeighbour->PrevOffset = header->NextOffset;
            }
        }

        AllocationHeader* prevAllocation = PrevAllocation(header);
        if (prevAllocation != NULL && IISBITSET(prevAllocation->Flags, AllocationHeader::FreeFlagBit))
        {
            IVERIFY(VerifyAllocation(prevAllocation));
            ClearCanary(header);

            if (header->NextOffset == 0)
            {
                prevAllocation->NextOffset = 0;
            }
            else
            {
                prevAllocation->NextOffset += header->NextOffset;

                if (nextNeighbour != NULL)
                {
                    nextNeighbour->PrevOffset = prevAllocation->NextOffset;
                }
                else if (nextAllocation != NULL && !IISBITSET(nextAllocation->Flags, AllocationHeader::FreeFlagBit))
                {
                    nextAllocation->PrevOffset = prevAllocation->NextOffset;
                }
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
    void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        const bool isNull = (uintptr_t)a_ptr < sizeof(AllocationHeader);
        if (isNull)
        {
            return Allocate(a_size, a_alignment);
        }

        void* headerPtr = (char*)a_ptr - sizeof(AllocationHeader);
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

// MIT License
// 
// Copyright (c) 2025 River Govers
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