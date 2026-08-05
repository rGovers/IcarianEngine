// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/Allocator.h"

ICARIAN_PUSH_FASTALLOCTOR

namespace IcarianCore
{
    class UberAllocator
    {
    private:
        // At this big the number of memory pages should never be an issue
        // (Can still be as allocators tend to directly hit the OS for large allocations)
        // We can afford a massive chunk as this is to divy up to allocators and not allocated directly
        // Yes this means we allocate HUGE chunks of memory at once but we are a game so we expect a decent amount of memory usage
        // We take out a KiB to leave room for overhead so we do not overrun a page boundary and trigger a rounding up to the next power of 2
        // TODO: May want to make the size of this platform specific as for desktop 256MiB at a time is fine but other platforms this may be an issue
        constexpr static uint32_t UberAllocatorBlockSize = (256 << 20) - (1 << 10);

    protected:

    public:
        // OS only has a limited number of memory pages so to get around that we just allocate a larger block then divy it up to the allocators
        // We can do this as the OS should support large pages
        //
        // We needed to do this as with the custom allocator we gave to the Mono runtime we where bombarding the Kernel for pages
        // Should have saw that coming as managed languages love allocating memory
        static Allocator* Instance;

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
