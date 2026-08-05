// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/ComplexAllocator.h"

#include <atomic>

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    class TrackerAllocator : public ComplexAllocator
    {
    private:
        struct AllocationHeader
        {
    #ifdef DEBUG
            uintptr_t CanaryA;
    #endif
            uint64_t Size;
            uint32_t BaseOffset;
            uint32_t Alignment;
    #ifdef DEBUG
            uintptr_t CanaryB;
    #endif
        };

        constexpr static uintptr_t CanaryValue = 0xCCCCCCCCCCCCCCCC;

        Allocator*            m_upstreamAllocator;

        std::atomic<uint64_t> m_memoryUsage;
        std::atomic<uint64_t> m_trueMemoryUsage;

        constexpr static AllocationHeader* AllocationFromPointer(const void* a_ptr)
        {
            return (AllocationHeader*)((uintptr_t)a_ptr - sizeof(AllocationHeader));
        }

        static void SetCanary(AllocationHeader* a_header);
        static void VerifyAllocation(const AllocationHeader* a_header);

    protected:

    public:
        TrackerAllocator(Allocator* a_upstreamAllocator);
        virtual ~TrackerAllocator();

        inline uint64_t GetMemoryUsage() const
        {
            return m_memoryUsage;
        }
        inline uint64_t GetTrueMemoryUsage() const
        {
            return m_trueMemoryUsage;
        }

        inline Allocator* GetUpstreamAllocator() const
        {
            return m_upstreamAllocator;
        }

        [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment);
        virtual void Free(void* a_ptr);
        [[nodiscard]] virtual void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment);
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
