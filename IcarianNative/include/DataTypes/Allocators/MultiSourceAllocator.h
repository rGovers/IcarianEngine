// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocators/ComplexAllocator.h"

#include "Core/IcarianDefer.h"
#include "IcarianError.h"
#include "IcarianMemory.h"

struct AllocationSource
{
    Allocator* Alloc;
    uint64_t MaxSize;
};

class MultiSourceAllocator : public ComplexAllocator
{
private:
    struct AllocationHeader
    {
#ifdef DEBUG
        uintptr_t CanaryA;
#endif
        uint32_t AllocatorIndex;
        uint32_t BaseOffset;
        uint64_t Size;
#ifdef DEBUG
        uintptr_t CanaryB;
#endif
    };

    constexpr static uint64_t CanaryValue = 0xCCCCCCCCCCCCCCCC;

    Allocator*        m_mainAllocator;

    AllocationSource* m_sources;
    uint32_t          m_sourceCount;

    static AllocationHeader* AllocationFromPointer(const void* a_ptr)
    {
        return (AllocationHeader*)((uint8_t*)a_ptr - sizeof(AllocationHeader));
    }

    static void SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;
#endif
    }

    void VerifyAllocation(const AllocationHeader* a_header)
    {
#ifdef DEBUG
        IVERIFY(a_header->CanaryA == CanaryValue && a_header->CanaryB == CanaryValue);

        IVERIFY(a_header->AllocatorIndex < m_sourceCount);
#endif
    }

protected:

public:
    MultiSourceAllocator(Allocator* a_mainAllocator, const AllocationSource* a_sources, uint32_t a_sourceCount)
    {
        m_mainAllocator = a_mainAllocator;

        m_sourceCount = a_sourceCount;
        m_sources = m_mainAllocator->TAllocate<AllocationSource>(m_sourceCount);
        for (uint32_t i = 0; i < m_sourceCount; ++i)
        {
            m_sources[i] = a_sources[i];
        }
    }
    virtual ~MultiSourceAllocator()
    {
        m_mainAllocator->Free(m_sources);
    }

    [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        const uint64_t size = a_size + sizeof(AllocationHeader) + a_alignment;
        for (uint32_t i = 0; i < m_sourceCount; ++i)
        {
            if (size >= m_sources[i].MaxSize)
            {
                continue;
            }

            void* basePtr = m_sources[i].Alloc->Allocate(size, BaseAlignment);
            void* allocationPtr = AlignTo((uint8_t*)basePtr + sizeof(AllocationHeader), a_alignment);

            AllocationHeader* header = AllocationFromPointer(allocationPtr);
            SetCanary(header);
            header->AllocatorIndex = i;
            header->Size = a_size;
            header->BaseOffset = (uint32_t)((uint8_t*)header - (uint8_t*)basePtr);

            return allocationPtr;
        }

        IERROR("Allocation failed");

        return nullptr;
    }
    virtual void Free(void* a_ptr)
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
    [[nodiscard]] virtual void* Realloc(void* a_ptr, uint64_t a_size, uint32_t a_alignment)
    {
        if (a_ptr == nullptr)
        {
            return Allocate(a_size, a_alignment);
        }

        AllocationHeader* header = AllocationFromPointer(a_ptr);
        VerifyAllocation(header);

        void* basePtr = (uint8_t*)header - header->BaseOffset;
        Allocator* allocator = m_sources[header->AllocatorIndex].Alloc;
        IDEFER(allocator->Free(basePtr));

        void* newPtr = Allocate(a_size, a_alignment);
        memcpy(newPtr, a_ptr, header->Size);

        return newPtr;
    }
};

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