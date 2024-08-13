// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/ThreadGuard.h"

template<typename T, typename TMutex = SharedSpinLock, typename TLock = ThreadGuard<SharedSpinLock>>
class TLockArray
{
private:
    TLock*   m_lock;
    T*       m_data;
    uint32_t m_size;

protected:

public:
    typedef T* iterator;
    typedef const T* const_iterator;

    TLockArray() = delete;
    TLockArray(const TLockArray&) = delete;
    TLockArray(TLockArray&& a_other) noexcept
    {
        m_lock = a_other.m_lock;
        m_data = a_other.m_data;
        m_size = a_other.m_size;

        a_other.m_lock = nullptr;
        a_other.m_size = 0;
        a_other.m_data = nullptr;
    }
    explicit TLockArray(TMutex& a_mutex)
    {
        m_lock = new TLock(a_mutex);

        m_data = nullptr;
        m_size = 0;
    }
    ~TLockArray()
    {
        m_size = 0;
        m_data = nullptr;

        if (m_lock != nullptr)
        {
            delete m_lock;
        }
    }

    iterator begin() 
    {
        return m_data;
    }
    const_iterator begin() const
    {
        return m_data;
    }

    iterator end() 
    {
        return m_data + m_size;
    }
    const_iterator end() const
    {
        return m_data + m_size;
    }

    void SetData(T* a_data, uint32_t a_size)
    {
        m_data = a_data;
        m_size = a_size;
    }

    inline uint32_t Size() const
    {
        return m_size;
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
        m_data[a_index] = a_value;
    }
};

template<typename T>
using TReadLockArray = TLockArray<T, SharedSpinLock, SharedThreadGuard>;

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