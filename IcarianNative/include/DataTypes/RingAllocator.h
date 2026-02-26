// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocator.h"

#include "Core/IcarianDefer.h"
#include "IcarianMemory.h"

// A no deallocation allocator
// Loops back to the start when it runs out of memory
// Note that this allocator has no bounds checking or sanitizer so overflows will write to future allocations
// Not to be used as a main allocator used in performance critical areas where allocations are known to be small, frequent and short lived where general purpose allocators are too slow
// People forget that there are 100s of ways to allocate memory and you do not need to pick just one
class RingAllocator : public Allocator
{
private:
    void* m_memory;
    void* m_slider;
    void* m_end;

protected:

public:
    RingAllocator(uint64_t a_size)
    {
        m_memory = MapMemory(a_size);
        m_slider = m_memory;
        m_end = (void*)((char*)m_memory + a_size);
    }
    virtual ~RingAllocator()
    {
        UnmapMemory(m_memory, GetSize());
    }

    inline uint64_t GetSize() const
    {
        return (uint64_t)((char*)m_end - (char*)m_memory);
    }

    virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        void* next = AlignTo((uint8_t*)m_slider + a_size, (uintptr_t)a_alignment);

        if (next >= (uint8_t*)m_end)
        {
            m_slider = m_memory;
        }

        IDEFER(m_slider = (uint8_t*)m_slider + a_size);

        return m_slider;
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