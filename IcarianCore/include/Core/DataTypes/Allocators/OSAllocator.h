// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/Allocator.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    class TrackerAllocator;

    class OSAllocator : public Allocator
    {
    private:
        constexpr static uint64_t CanaryValue = 0xCCCCCCCCCCCCCCCC;

        struct AllocationHeader
        {
    #ifdef DEBUG
            uintptr_t CanaryA;
    #endif
            void* Ptr;
            uint64_t Size;
    #ifdef DEBUG
            uintptr_t CanaryB;
    #endif
        };

        constexpr static AllocationHeader* AllocationFromPointer(const void* a_ptr)
        {
            return (AllocationHeader*)((uintptr_t)a_ptr - sizeof(AllocationHeader));
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

        static void SetCanary(AllocationHeader* a_header);
        static void ClearCanary(AllocationHeader* a_header);

    protected:

    public:
        OSAllocator() { }
        ~OSAllocator() { }

        [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment);
        virtual void Free(void* a_ptr);

        static Allocator* Instance;
        static TrackerAllocator* TrackerInstance;

        static void Init();
        static void Destroy();
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
