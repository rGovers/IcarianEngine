// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocators/ComplexAllocator.h"

#include <cstdlib>

#include "IcarianMemory.h"

class TrackerAllocator;

ICARIAN_PUSH_FASTALLOCTOR

class MallocAllocator : public Allocator
{
private:

protected:

public:
    [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        // aligned_alloc requires that size be a multiple of alignment which is annoying as we just want it on the alignment boundary
        // To get around this we just round up the size if needed
        const uint64_t alignedSize = AlignTo(a_size, a_alignment);

        return aligned_alloc((size_t)a_alignment, (size_t)alignedSize);
    }

    virtual void Free(void* a_ptr)
    {
        free(a_ptr);
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