// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/ComplexAllocator.h"

#ifdef __linux__
#if defined (__GNUC__) && !defined (__clang__)
#include <cxxabi.h>
#endif
#endif

#include <cstring>

#include "Core/Bitfield.h"
#include "Core/DataTypes/SpinLock.h"
#include "Core/IcarianMemory.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
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

        constexpr static AllocationHeader* AllocationFromPointer(const void* a_ptr)
        {
            return (AllocationHeader*)((uintptr_t)a_ptr - sizeof(AllocationHeader));
        }

        constexpr static AllocationHeader* GetFirstAllocation(const BlockHeader* a_header)
        {
            if (a_header->First != NULL)
            {
                return a_header->First;
            }

            const void* allocPtr = AlignTo((uint8_t*)a_header + sizeof(BlockHeader), alignof(AllocationHeader));
            return (AllocationHeader*)allocPtr;
        }

        constexpr static AllocationHeader* NextAllocation(const AllocationHeader* a_header)
        {
            if (a_header->NextOffset == 0)
            {
                return NULL;
            }

            return (AllocationHeader*)((uint8_t*)a_header + a_header->NextOffset);
        }
        constexpr static AllocationHeader* PrevAllocation(const AllocationHeader* a_header)
        {
            if (a_header->PrevOffset == 0)
            {
                return NULL;
            }

            return (AllocationHeader*)((uint8_t*)a_header - a_header->PrevOffset);
        }

        constexpr static bool VerifyAllocation(const AllocationHeader* a_header)
        {
    #ifdef DEBUG
            if (a_header->CanaryA != CanaryValue || a_header->CanaryB != CanaryValue)
            {
                return false;
            }
    #endif

            return true;
        }

        constexpr static bool IsBlockFree(const BlockHeader* a_header)
        {
            const AllocationHeader* allocHeader = GetFirstAllocation(a_header);
            while (allocHeader != NULL)
            {
                if (!IISBITSET(allocHeader->Flags, AllocationHeader::FreeFlagBit))
                {
                    return false;
                }

                allocHeader = NextAllocation(allocHeader);
            }

            return true;
        }

        static void SetHeaderOffsets(AllocationHeader* a_prevHeader, AllocationHeader* a_nextHeader);
        static void SetCanary(AllocationHeader* a_header);
        static void ClearCanary(AllocationHeader* a_header);

        static void VerifyBlock(const BlockHeader* a_header);

        BlockHeader* AllocateBlock();
        void FreeBlock(BlockHeader* a_block);

    protected:

    public:
        BlockAllocator(uint32_t a_blockSize, Allocator* a_upstreamAllocator);
        virtual ~BlockAllocator();

        inline uint32_t GetBlockSize() const
        {
            return m_blockSize;
        }

        inline Allocator* GetUpstreamAllocator() const
        {
            return m_upstreamAllocator;
        }

        [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment);
        virtual void Free(void* a_ptr);
        // Have the data to implement realloc so just do it
        // NOTE: This is not thread safe
        [[nodiscard]] virtual void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment);

        void TrimBlocks();
    };
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
