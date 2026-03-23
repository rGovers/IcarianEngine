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
#include <cstring>
#include <type_traits>
#include <utility>

#include "Core/IcarianLambda.h"

class Allocator
{
private:

protected:

public:
    virtual ~Allocator() { }

    constexpr static uintptr_t BaseAlignment = 16;

    [[nodiscard]] virtual void* Allocate(uint64_t a_size, uint32_t a_alignment) = 0;
    virtual void Free(void* a_ptr) { }

    [[nodiscard]] virtual void* ZAllocate(uint64_t a_size, uint32_t a_alignment)
    {
        void* ptr = Allocate(a_size, a_alignment);
        if (ptr != NULL)
        {
            memset(ptr, 0, (size_t)a_size);
        }

        return ptr;
    }

    template<typename T>
    [[nodiscard]] T* TAllocate()
    {
        return (T*)Allocate(sizeof(T), alignof(T));
    }
    template<typename T>
    [[nodiscard]] T* TAllocate(uint64_t a_count)
    {
        return (T*)Allocate(sizeof(T) * a_count, alignof(T));
    }

    template<typename T>
    [[nodiscard]] T* ZTAllocate()
    {
        T* ptr = (T*)Allocate(sizeof(T), alignof(T));
        if (ptr != NULL)
        {
            memset((void*)ptr, 0, sizeof(T));
        }

        return ptr;
    }
    template<typename T>
    [[nodiscard]] T* ZTAllocate(uint64_t a_count)
    {
        const uint64_t size = sizeof(T) * a_count;

        T* ptr = (T*)Allocate(size, alignof(T));
        if (ptr != NULL)
        {
            memset((void*)ptr, 0, (size_t)size);
        }

        return ptr;
    }

    template<typename T, typename ... Args>
    [[nodiscard]] T* Create(Args&&... a_args)
    {
        // I forget that this syntax exists every time to do an in place constructor on an existing memory address
        return new (ZAllocate(sizeof(T), alignof(T))) T(std::forward<Args>(a_args)...);
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