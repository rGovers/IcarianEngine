// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/Allocator.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#elif defined(__linux__)
#include <sys/mman.h>
#else
#include <cstdlib>
#endif

#include "Core/IcarianDefer.h"
#include "IcarianError.h"

class StackAllocator : public Allocator
{
private:
    void* m_memory;
    void* m_end;
    void* m_stackPointer;
    void* m_stackSlider;

protected:

public:
    StackAllocator(uint64_t a_size)
    {
#ifdef WIN32
        m_memory = VirtualAlloc(nullptr, a_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif defined(__linux__)
        m_memory = mmap(nullptr, a_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
        // Fall back to malloc
        m_memory = malloc(a_size);
#endif
        m_end = (char*)m_memory + a_size;
        m_stackPointer = m_memory;
        m_stackSlider = m_memory;

    }
    ~StackAllocator()
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
    inline uint64_t GetUsedSize() const
    {
        return (uint64_t)((char*)m_stackSlider - (char*)m_memory);
    }
    inline uint64_t GetRemainingSize() const
    {
        return (uint64_t)((char*)m_end - (char*)m_stackSlider);
    }

    void PushStackPointer()
    {
        const uintptr_t oldStackSlider = (uintptr_t)m_stackSlider;

        uintptr_t* oldPtr = TAllocate<uintptr_t>();
        *oldPtr = (uintptr_t)m_stackPointer;

        m_stackPointer = m_stackSlider;

        IVERIFY((uintptr_t)m_stackSlider - oldStackSlider >= sizeof(uintptr_t));
    }
    void PopStackPointer()
    {
        if (m_stackPointer <= m_memory)
        {
            IERROR("Pop at bottom of the Stack");
        }

        m_stackSlider = m_stackPointer;
        m_stackPointer = (void*)*((uintptr_t*)m_stackPointer - 1);
    }

    inline void Reset()
    {
        m_stackPointer = m_memory;
        m_stackSlider = m_memory;
    }

    virtual void* Allocate(uint64_t a_size, uint32_t a_alignment)
    {
        void* ptr = Align(m_stackSlider, a_alignment);
        void* next = (char*)ptr + a_size;
        if (next > m_end)
        {
            IERROR("Stack Allocator out of memory");
        }

        IDEFER(m_stackSlider = next);

#ifdef DEBUG
        const uintptr_t off = (char*)m_end - (char*)m_memory;
        if (next > (char*)m_memory + (uintptr_t)(off * 0.9))
        {
            Logger::Warning("High Stack memory usage!");
        }
#endif

        return ptr;
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