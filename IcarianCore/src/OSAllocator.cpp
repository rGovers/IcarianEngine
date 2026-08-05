// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/DataTypes/Allocators/OSAllocator.h"

#include "Core/DataTypes/Allocators/BlockAllocator.h"
#include "Core/DataTypes/Allocators/LeakAllocator.h"
#include "Core/DataTypes/Allocators/TrackerAllocator.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    Allocator* OSAllocator::Instance = nullptr;
    TrackerAllocator* OSAllocator::TrackerInstance = nullptr;

    // Need to unwind in a specific order so need to keep track of everything
    static OSAllocator* InternalOSAllocator = nullptr;
    static BlockAllocator* InternalBlockAllocator = nullptr;
    static LeakAllocator* InternalLeakAllocator = nullptr;

    void OSAllocator::SetCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = CanaryValue;
        a_header->CanaryB = CanaryValue;
#endif
    }
    void OSAllocator::ClearCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = 0;
        a_header->CanaryB = 0;
#endif
    }

    void* OSAllocator::Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        if (a_size <= 0)
        {
            return nullptr;
        }

        const uint32_t targetAlignment = ILAMBDA(
        {
            if (a_alignment > alignof(AllocationHeader))
            {
                ILRETURN AlignTo(a_alignment, alignof(AllocationHeader));
            }

            ILRETURN (uint32_t)alignof(AllocationHeader);
        });

        const uint64_t size = AlignTo(a_size + sizeof(AllocationHeader), targetAlignment);

#ifdef WIN32
        void* basePtr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif defined(__linux__)
        // Seem to be having funkyness with pages so not gonna do anything fancy and just let the Kernel figure out the rest as that seems to be working
        void* basePtr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (basePtr == (void*)-1)
        {
            // We are not malloc if we fail at this stage no point retrying just fail and leave
            // Not gonna implement clever scavenging just if we are out we are out
            basePtr = nullptr;
        }
#else
        // Fall back to malloc
        void* basePtr = malloc(size);
#endif
        if (basePtr == nullptr)
        {
            return nullptr;
        }

        void* alignedPtr = AlignTo((uint8_t*)basePtr + sizeof(AllocationHeader), targetAlignment);
        AllocationHeader* header = AllocationFromPointer(alignedPtr);
        SetCanary(header);
        header->Size = a_size;
        header->Ptr = basePtr;

        return alignedPtr;
    }
    void OSAllocator::Free(void* a_ptr)
    {
        AllocationHeader* header = AllocationFromPointer(a_ptr);
        ICARIAN_ASSERT(VerifyAllocation(header));

        ClearCanary(header);
#ifdef WIN32
        VirtualFree(header->Ptr, header->Size, MEM_RELEASE);
#elif defined(__linux__)
        munmap(header->Ptr, header->Size);
#else
        free(header->Ptr);
#endif
    }

    void OSAllocator::Init()
    {
        if (Instance == nullptr)
        {
            OSAllocator* osAlloc = new OSAllocator();
            InternalOSAllocator = osAlloc;

            TrackerAllocator* tracker = new TrackerAllocator(osAlloc);
            TrackerInstance = tracker;
            Instance = tracker;

    #ifdef DEBUG
            InternalBlockAllocator = new BlockAllocator(8 << 10, Instance);
            InternalLeakAllocator = new LeakAllocator(Instance, InternalBlockAllocator);
            Instance = InternalLeakAllocator;
    #endif
        }
    }
    void OSAllocator::Destroy()
    {
        if (Instance != nullptr)
        {
            if (InternalLeakAllocator != nullptr)
            {
                delete InternalLeakAllocator;
                InternalLeakAllocator = nullptr;
            }

            if (InternalBlockAllocator != nullptr)
            {
                delete InternalBlockAllocator;
                InternalBlockAllocator = nullptr;
            }

            delete TrackerInstance;
            TrackerInstance = nullptr;

            delete InternalOSAllocator;
            InternalOSAllocator = nullptr;

            Instance = nullptr;
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
