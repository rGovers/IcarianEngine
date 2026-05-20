// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocators/Allocator.h"

#include "DataTypes/Allocators/TrackerAllocator.h"
#include "IcarianError.h"
#include "IcarianMemory.h"

ICARIAN_PUSH_FASTALLOCTOR

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
    static void ClearCanary(AllocationHeader* a_header)
    {
#ifdef DEBUG
        a_header->CanaryA = 0;
        a_header->CanaryB = 0;
#endif
    }

protected:

public:
    OSAllocator() { }
    ~OSAllocator() { }

    [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        const uint64_t size = a_size + a_alignment + sizeof(AllocationHeader);

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

        void* alignedPtr = AlignTo((uint8_t*)basePtr + sizeof(AllocationHeader), a_alignment);
        AllocationHeader* header = AllocationFromPointer(alignedPtr);
        SetCanary(header);
        header->Size = a_size;
        header->Ptr = basePtr;

        return alignedPtr;
    }
    virtual void Free(void* a_ptr)
    {
        AllocationHeader* header = AllocationFromPointer(a_ptr);
        IVERIFY(VerifyAllocation(header));

        ClearCanary(header);
#ifdef WIN32
        VirtualFree(header->Ptr, header->Size, MEM_RELEASE);
#elif defined(__linux__)
        munmap(header->Ptr, header->Size);
#else
        free(header->Ptr);
#endif
    }

    static Allocator* Instance;
    static TrackerAllocator* TrackerInstance;

    static void Init();
    static void Destroy();
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