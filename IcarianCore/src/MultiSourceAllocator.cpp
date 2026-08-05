// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/MultiSourceAllocator.h"

#include "Core/IcarianDefer.h"
#include "Core/IcarianMemory.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    MultiSourceAllocator::MultiSourceAllocator(Allocator* a_mainAllocator, const AllocationSource* a_sources, uint32_t a_sourceCount)
    {
        m_mainAllocator = a_mainAllocator;

        m_sourceCount = a_sourceCount;
        m_sources = m_mainAllocator->TAllocate<AllocationSource>(m_sourceCount);
        for (uint32_t i = 0; i < m_sourceCount; ++i)
        {
            m_sources[i] = a_sources[i];
        }
    }
    MultiSourceAllocator::~MultiSourceAllocator()
    {
        m_mainAllocator->Free(m_sources);
    }

    void MultiSourceAllocator::SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;
#endif
    }

    void MultiSourceAllocator::VerifyAllocation(const AllocationHeader* a_header)
    {
#ifdef DEBUG
        ICARIAN_ASSERT(a_header->CanaryA == CanaryValue);
        ICARIAN_ASSERT(a_header->CanaryB == CanaryValue);

        ICARIAN_ASSERT(a_header->AllocatorIndex < m_sourceCount);
#endif
    }

    void* MultiSourceAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        // Find an alignment that fits both the header and the allocation alignment
        const uint32_t targetAlignment = ILAMBDA(
        {
            if (a_alignment > alignof(AllocationHeader))
            {
                ILRETURN AlignTo(a_alignment, alignof(AllocationHeader));
            }

            ILRETURN (uint32_t)alignof(AllocationHeader);
        });

        const uint64_t size = AlignTo(a_size + sizeof(AllocationHeader), targetAlignment) + targetAlignment;
        for (uint32_t i = 0; i < m_sourceCount; ++i)
        {
            if (size >= m_sources[i].MaxSize)
            {
                continue;
            }

            void* basePtr = m_sources[i].Alloc->Allocate(size, alignof(AllocationHeader));
            void* allocationPtr = AlignTo((uint8_t*)basePtr + sizeof(AllocationHeader), targetAlignment);

            AllocationHeader* header = AllocationFromPointer(allocationPtr);
            SetCanary(header);
            header->AllocatorIndex = i;
            header->Size = a_size;
            header->BaseOffset = (uint32_t)((uint8_t*)header - (uint8_t*)basePtr);

            return allocationPtr;
        }

        ICARIAN_ASSERT(0);

        return nullptr;
    }
    void MultiSourceAllocator::Free(void* a_ptr)
    {
        if (a_ptr == nullptr)
        {
            return;
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        VerifyAllocation(header);

        void* basePtr = (uint8_t*)header - header->BaseOffset;
        Allocator* allocator = m_sources[header->AllocatorIndex].Alloc;

        allocator->Free(basePtr);
    }
    void* MultiSourceAllocator::Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        VerifyAllocation(header);
        if (header->Size >= a_size && IsAligned((uint8_t*)a_ptr, (uintptr_t)a_alignment))
        {
            return a_ptr;
        }

        void* basePtr = (uint8_t*)header - header->BaseOffset;
        Allocator* allocator = m_sources[header->AllocatorIndex].Alloc;
        IDEFER(allocator->Free(basePtr));

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
