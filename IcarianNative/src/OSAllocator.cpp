// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "DataTypes/Allocators/OSAllocator.h"

#include "DataTypes/Allocators/BlockAllocator.h"
#include "DataTypes/Allocators/LeakAllocator.h"

Allocator* OSAllocator::Instance = nullptr;
TrackerAllocator* OSAllocator::TrackerInstance = nullptr;

// Need to unwind in a specific order so need to keep track of everything
static OSAllocator* InternalOSAllocator = nullptr;
static BlockAllocator* InternalBlockAllocator = nullptr;
static LeakAllocator* InternalLeakAllocator = nullptr;

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