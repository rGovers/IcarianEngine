// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "DataTypes/Allocators/OSAllocator.h"

#include "DataTypes/Allocators/BlockAllocator.h"
#include "DataTypes/Allocators/LeakAllocator.h"

Allocator* OSAllocator::Instance = nullptr;

void OSAllocator::Init()
{
    if (OSAllocator::Instance == nullptr)
    {
        OSAllocator::Instance = new OSAllocator();

#ifdef DEBUG
        // Preferably do not want he LeakDetector to run on raw pages so allocate a BlockAllocator for it
        // Still want a LeakAllocator to see if we leak memory pages we get from the OS
        // This leak detector should never trigger but if it does it likely means there is a broken Allocator implementation in the codebase
        BlockAllocator* blockAllocator = new BlockAllocator(8 << 10, OSAllocator::Instance);

        OSAllocator::Instance = new LeakAllocator(OSAllocator::Instance, blockAllocator);
#endif
    }
}
void OSAllocator::Destroy()
{
    if (OSAllocator::Instance != nullptr)
    {
#ifdef DEBUG
        LeakAllocator* leakAllocator = (LeakAllocator*)OSAllocator::Instance;

        Allocator* upstreamAllocator = leakAllocator->GetUpstreamAllocator();
        IDEFER(delete upstreamAllocator);

        Allocator* storageAllocator = leakAllocator->GetStorageAllocator();
        IDEFER(delete storageAllocator);
#endif

        delete OSAllocator::Instance;
        OSAllocator::Instance = nullptr;
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