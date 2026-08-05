// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/TrackerAllocator.h"

#include "Core/IcarianDefer.h"
#include "Core/IcarianMemory.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    TrackerAllocator::TrackerAllocator(Allocator* a_upstreamAllocator)
    {
        m_upstreamAllocator = a_upstreamAllocator;

        m_memoryUsage = 0;
        m_trueMemoryUsage = 0;
    }
    TrackerAllocator::~TrackerAllocator()
    {

    }

    void TrackerAllocator::SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;
#endif
    }

    void TrackerAllocator::VerifyAllocation(const AllocationHeader* a_header)
    {
#ifdef DEBUG
        ICARIAN_ASSERT(a_header->CanaryA == CanaryValue);
        ICARIAN_ASSERT(a_header->CanaryB == CanaryValue);
#endif
    }

    void* TrackerAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        m_memoryUsage.fetch_add(a_size);

        const uint32_t targetAlignment = ILAMBDA(
        {
            if (a_alignment > alignof(AllocationHeader))
            {
                ILRETURN AlignTo(a_alignment, alignof(AllocationHeader));
            }

            ILRETURN (uint32_t)alignof(AllocationHeader);
        });

        const uint64_t trueSize = AlignTo(a_size + sizeof(AllocationHeader), targetAlignment) + targetAlignment;
        m_trueMemoryUsage.fetch_add(trueSize);

        void* basePtr = m_upstreamAllocator->Allocate(trueSize, alignof(AllocationHeader));
        void* allocationPtr = AlignTo((uint8_t*)basePtr + sizeof(AllocationHeader), targetAlignment);

        AllocationHeader* header = AllocationFromPointer(allocationPtr);
        SetCanary(header);
        header->Size = a_size;
        header->Alignment = a_alignment;
        header->BaseOffset = (uint32_t)((uint8_t*)header - (uint8_t*)basePtr);

        return allocationPtr;
    }
    void TrackerAllocator::Free(void* a_ptr)
    {
        if (a_ptr == nullptr)
        {
            return;
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        VerifyAllocation(header);

        const uint32_t targetAlignment = ILAMBDA(
        {
            if (header->Alignment > alignof(AllocationHeader))
            {
                ILRETURN AlignTo(header->Alignment, alignof(AllocationHeader));
            }

            ILRETURN (uint32_t)alignof(AllocationHeader);
        });

        const uint64_t trueSize = AlignTo(header->Size + sizeof(AllocationHeader), targetAlignment) + targetAlignment;

        m_memoryUsage.fetch_sub(header->Size);
        m_trueMemoryUsage.fetch_sub(trueSize);

        void* basePtr = (uint8_t*)header - header->BaseOffset;
        m_upstreamAllocator->Free(basePtr);
    }
    void* TrackerAllocator::Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        VerifyAllocation(header);

        void* basePtr = (uint8_t*)header - header->BaseOffset;
        IDEFER(m_upstreamAllocator->Free(basePtr));

        void* newPtr = Allocate(a_size, a_alignment);
        memcpy(newPtr, a_ptr, header->Size);

        return newPtr;
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
