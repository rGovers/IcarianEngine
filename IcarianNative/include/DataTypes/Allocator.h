// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#elif defined(__linux__)
#include <execinfo.h>
#include <sys/mman.h>
#else
#include <cstdlib>
#endif

#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>

struct MallocAllocator
{
    static void* Allocate(uint64_t a_value, uint32_t a_alignment)
    {
        return malloc(a_value);
    }
    static void Free(void* a_ptr)
    {
        free(a_ptr);
    }
};

class Allocator
{
private:
    constexpr static uint64_t UnixPageSize = 4 << 10;

protected:
    static void* MapMemory(uint64_t a_size)
    {
#ifdef WIN32
        return VirtualAlloc(NULL, a_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif defined(__linux__)
        // Seem to be having funkyness with pages so not gonna do anything fancy and just let the Kernel figure out the rest as that seems to be working
        void* ptr = mmap(NULL, a_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == (void*)-1)
        {
            // We are not malloc if we fail at this stage no point retrying just fail and leave
            // Not gonna implement clever scavenging just if we are out we are out
            return nullptr;
        }

        return ptr;
#else
        // Fall back to malloc
        return malloc(a_size);
#endif
    }
    static void UnmapMemory(void* a_ptr, uint64_t a_size)
    {
#ifdef WIN32
        VirtualFree(a_ptr, a_size, MEM_RELEASE);
#elif defined(__linux__)
        munmap(a_ptr, a_size);
#else
        free(m_memory);
#endif
    }

public:
    virtual ~Allocator() { }

    constexpr static uint32_t BaseAlignment = sizeof(void*);

    // C++ spec has this as undefined behaviour to my knowledge so alignment is necessary
    static void* Align(const void* a_ptr, uint32_t a_alignment)
    {
        uint32_t alignOffset = (uintptr_t)a_ptr % a_alignment;
        if (alignOffset != 0)
        {
            alignOffset = a_alignment - alignOffset;
        }

        return (char*)a_ptr + alignOffset;
    }

    virtual void* Allocate(uint64_t a_size, uint32_t a_alignment) = 0;
    virtual void Free(void* a_ptr) { }

    template<typename T>
    T* TAllocate()
    {
        return (T*)Allocate(sizeof(T), alignof(T));
    }
    template<typename T>
    T* TAllocate(uint64_t a_count)
    {
        return (T*)Allocate(sizeof(T) * a_count, alignof(T));
    }

    template<typename T, typename ... Args>
    T* Create(Args&&... a_args)
    {
        // I forget that this syntax exists every time to do an in place constructor on an existing memory address
        return new (Allocate(sizeof(T), alignof(T))) T(std::forward<Args>(a_args)...);
    }

    template<typename T>
    void Destroy(T* a_ptr)
    {
        if constexpr (!std::is_trivially_destructible<T>())
        {
            a_ptr->~T();   
        }

        Free(a_ptr);
    }
};

// MIT License
// 
// Copyright (c) 2025 River Govers
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