// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include "Core/IcarianDefer.h"
#include "DataTypes/Allocators/Allocator.h"
#include "IcarianError.h"

// This only exists because they STL only dictates the interface and not how it is implemented
// This should be more predictable then the std::vector on different platforms
template<typename T>
class Array
{
private:
    Allocator* m_allocator;

    T*         m_data;
    uint32_t   m_size;
    uint32_t   m_capacity;

    inline void DestroyData()
    {
        if constexpr (!std::is_trivially_destructible<T>())
        {
            for (uint32_t i = 0; i < m_size; ++i)
            {
                T* dat = &(m_data[i]);

                dat->~T();
            }
        }

        memset((void*)m_data, 0, m_capacity * sizeof(T));
    }

protected:

public:
    typedef T* iterator;
    typedef const T* const_iterator;

    Array(Allocator* a_allocator)
    {
        IVERIFY(a_allocator != nullptr);
        m_allocator = a_allocator;

        m_size = 0;
        m_capacity = 1;
        m_data = m_allocator->ZTAllocate<T>(m_capacity);
    }
    Array(const Array& a_other) : Array(a_other.m_data, a_other.m_size, a_other.m_allocator) { }
    Array(const Array& a_other, Allocator* a_allocator) : Array(a_other.m_data, a_other.m_size, a_allocator) { }
    Array(const T* a_data, uint32_t a_size, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_size = a_size;
        m_capacity = a_size;
        if (m_capacity < 1)
        {
            m_capacity = 1;
        }

        m_data = m_allocator->ZTAllocate<T>(m_capacity);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            new (m_data + i) T(a_data[i]);
        }
    }
    ~Array()
    {
        if (m_data != NULL)
        {
            DestroyData();

            m_allocator->Free(m_data);
        }
    }

    inline Array& operator =(const Array& a_other)
    {
        if (m_data != NULL && m_allocator != NULL)
        {
            DestroyData();

            m_allocator->Free(m_data);
        }

        // This makes me uncomfortable but RAII is a bitch
        // The fun stuff you have to do when you use 1 memory pattern but dependencies use another
        m_allocator = a_other.m_allocator;
        m_size = a_other.m_size;
        m_capacity = a_other.m_size;

        if (m_capacity < 1)
        {
            m_capacity = 1;
        }

        m_data = m_allocator->ZTAllocate<T>(m_capacity);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            new (m_data + i) T(a_other.m_data[i]);
        }

        return *this;
    }

    inline iterator begin()
    {
        return m_data;
    }
    inline const_iterator begin() const
    {
        return m_data;
    }

    inline iterator end()
    {
        return m_data + m_size;
    }
    inline const_iterator end() const
    {
        return m_data + m_size;
    }

    inline Allocator* GetAllocator() const
    {
        return m_allocator;
    }

    inline uint32_t Size() const
    {
        return m_size;
    }
    inline uint32_t Capacity() const
    {
        return m_capacity;
    }
    inline bool Empty() const
    {
        return m_size == 0;
    }

    inline T* Data()
    {
        return m_data;
    }
    inline const T* Data() const
    {
        return m_data;
    }

    void Clear()
    {
        if (m_data != NULL)
        {
            DestroyData();
        }

        m_size = 0;
    }

    void Push(const T& a_data)
    {
        const uint32_t aSize = m_size + 1;
        IDEFER(m_size = aSize);

        if (aSize > m_capacity)
        {
            Reserve(m_capacity << 1);
        }

        new (m_data + m_size) T(a_data);
    }
    void Insert(uint32_t a_index, const T& a_data)
    {
        const uint32_t aSize = m_size + 1;
        IDEFER(m_size = aSize);

        if (aSize > m_capacity)
        {
            Reserve(m_capacity << 1);
        }

        memmove(m_data + a_index + 1, m_data + a_index, (m_size - a_index) * sizeof(T));

        new (m_data + a_index) T(a_data);
    }

    void Erase(uint32_t a_index)
    {
        const uint32_t aSize = m_size - 1;
        IDEFER(m_size = aSize);

        if constexpr (!std::is_trivially_destructible<T>())
        {
            // Today I learned that not just pointer values but pointers can be deconstructed 
            // Thank you for this knowledge debugger now I live in horror
            // What vodoo is happening that I can deconstruct a address what is there to deconstruct?!
            // TF it is a NOP what the actual fuck
            // Pointer deconstructors are NOP I want to cry it should not have even reached here
            // I dodged a bullet and am lucky this did not cause any issues I now have to account for this when using std::is_destructible
            // Should be fixed now with is_trivially_destructible
            (&(m_data[a_index]))->~T();
        }

        memmove(m_data + a_index, m_data + a_index + 1, (aSize - a_index) * sizeof(T));
        memset((void*)(m_data + aSize), 0, sizeof(T));
    }

    void Resize(uint32_t a_size)
    {
        IDEFER(m_size = a_size);

        Reserve(a_size);

        if constexpr (std::is_constructible<T>())
        {
            for (uint32_t i = m_size; i < a_size; ++i)
            {
                m_data[i] = T();
            }
        }
    }
    void Reserve(uint32_t a_size)
    {
        if (a_size > m_capacity)
        {
            IDEFER(m_capacity = a_size);

            T* newData = m_allocator->ZTAllocate<T>(a_size);
            memcpy((void*)newData, m_data, m_capacity * sizeof(T));
            m_allocator->Free(m_data);
            m_data = newData;
        }
    }

    inline T& operator [](uint32_t a_index)
    {
        return m_data[a_index];
    }
    inline const T& operator [](uint32_t a_index) const
    {
        return m_data[a_index];
    }

    inline T& Ref(uint32_t a_index)
    {
        return m_data[a_index];
    }
    inline T Get(uint32_t a_index) const
    {
        return m_data[a_index];
    }
    inline void Set(uint32_t a_index, const T& a_value)
    {
        new (m_data + a_index) T(a_value);
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