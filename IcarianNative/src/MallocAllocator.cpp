// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "DataTypes/Allocators/MallocAllocator.h"

#include "DataTypes/Allocators/LeakAllocator.h"
#include "DataTypes/Allocators/TrackerAllocator.h"

Allocator* MallocAllocator::Instance = nullptr;
TrackerAllocator* MallocAllocator::TrackerInstance = nullptr;

constexpr uint32_t AllocatorChainSize = 4;
static Allocator* AllocatorChain[AllocatorChainSize];

static void InsertAllocator(Allocator* a_allocator)
{
    for (uint32_t i = 0; i < AllocatorChainSize; ++i)
    {
        if (AllocatorChain[i] != nullptr)
        {
            continue;
        }

        AllocatorChain[i] = a_allocator;

        return;
    }

    IERROR("No more room for allocators in chain");
}

void MallocAllocator::Init()
{
    if (Instance == nullptr)
    {
        for (uint32_t i = 0; i < AllocatorChainSize; ++i)
        {
            AllocatorChain[i] = nullptr;
        }

        MallocAllocator* mallocAlloc = new MallocAllocator();
        InsertAllocator(mallocAlloc);

        TrackerAllocator* tracker = new TrackerAllocator(mallocAlloc);
        TrackerInstance = tracker;
        Instance = tracker;
        InsertAllocator(tracker);

#ifdef DEBUG
        LeakAllocator* leakAllocator = new LeakAllocator(Instance);
        Instance = leakAllocator;
        InsertAllocator(leakAllocator);
#endif
    }
}
void MallocAllocator::Destroy()
{
    if (Instance != nullptr)
    {
        for (uint32_t i = 0; i < AllocatorChainSize; ++i)
        {
            Allocator* alloc = AllocatorChain[AllocatorChainSize - i - 1];
            if (alloc == nullptr)
            {
                continue;
            }

            delete alloc;
        }

        Instance = nullptr;
    }
}

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