// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

#include "Core/IcarianDefer.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#elif defined(__linux__)
#include <sys/mman.h>
#else
#include <cstdlib>
#endif

// A no deallocation allocator
// Loops back to the start when it runs out of memory
// Note that this allocator has no bounds checking or sanitizer so overflows will write to future allocations
// Not to be used as a main allocator used in performance critical areas where allocations are known to be small, frequent and short lived where general purpose allocators are too slow
// People forget that there are 100s of ways to allocate memory and you do not need to pick just one
class RingAllocator
{
private:
    void* m_memory;
    void* m_slider;
    void* m_end;

protected:

public:
    RingAllocator(uint64_t a_size)
    {
#ifdef WIN32
        m_memory = VirtualAlloc(nullptr, a_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif defined(__linux__)
        m_memory = mmap(nullptr, a_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
        // Fall back to malloc
        m_memory = malloc(a_size);
#endif
        m_slider = m_memory;
        m_end = (void*)((char*)m_memory + a_size);
    }
    ~RingAllocator()
    {
#ifdef WIN32
        VirtualFree(m_memory, 0, MEM_RELEASE);
#elif defined(__linux__)
        munmap(m_memory, GetSize());
#else
        free(m_memory);
#endif
    }

    inline uint64_t GetSize() const
    {
        return (uint64_t)((char*)m_end - (char*)m_memory);
    }

    void* Allocate(uint64_t a_size)
    {
        if ((char*)m_slider + a_size > (char*)m_end)
        {
            m_slider = m_memory;
        }

        IDEFER(m_slider = (char*)m_slider + a_size);

        return m_slider;
    }
    template<typename T>
    T* Allocate()
    {
        return (T*)Allocate(sizeof(T));
    }
    template<typename T>
    T* Allocate(uint64_t a_count)
    {
        return (T*)Allocate(sizeof(T) * a_count);
    }
};

// MIT License
// 
// Copyright (c) 2024 River Govers
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