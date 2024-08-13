// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <cstdlib>

class ChunkAllocator
{
private:
    void* m_memory;
    void* m_slider;
    void* m_end;

protected:

public:
    ChunkAllocator(uint64_t a_size)
    {
        // Can probably use os specific allocation here eg. mmap on linux or VirtualAlloc on windows but lazy
        m_memory = malloc(a_size);
        m_slider = m_memory;
        m_end = (void*)((char*)m_memory + a_size);
    }
    ~ChunkAllocator()
    {
        free(m_memory);
    }

    inline uint64_t GetSize() const
    {
        return (uint64_t)((char*)m_end - (char*)m_memory);
    }
    inline uint64_t GetUsed() const
    {
        return (uint64_t)((char*)m_slider - (char*)m_memory);
    }
    inline uint64_t GetFree() const
    {
        return (uint64_t)((char*)m_end - (char*)m_slider);
    }

    void* Allocate(uint64_t a_size)
    {
        if ((char*)m_slider + a_size > (char*)m_end)
        {
            return nullptr;
        }

        void* result = m_slider;
        m_slider = (void*)((char*)m_slider + a_size);
        return result;
    }

    template<typename T>
    inline T* Allocate()
    {
        return (T*)Allocate(sizeof(T));
    }
    template<typename T>
    inline T* Allocate(uint64_t a_count)
    {
        return (T*)Allocate(sizeof(T) * a_count);
    }

    inline void Reset()
    {
        m_slider = m_memory;
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