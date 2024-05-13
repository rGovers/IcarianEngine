#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include "Core/IcarianDefer.h"

// This only exists because they STL only dictates the interface and not how it is implemented
// This should be more predictable then the std::vector on different platforms
template<typename T>
class Array
{
private:
    T*       m_data;
    uint32_t m_size;
    uint32_t m_capacity;

    inline void DestroyData()
    {
        if constexpr (std::is_destructible<T>())
        {
            for (uint32_t i = 0; i < m_size; ++i)
            {
                (&(m_data[i]))->~T();
            }
        }
    }

protected:

public:
    typedef T* iterator;
    typedef const T* const_iterator;

    Array()
    {
        m_size = 0;
        m_capacity = 1;
        m_data = (T*)calloc(1, sizeof(T));
    }
    Array(const Array& a_other)
    {
        m_size = a_other.m_size;
        m_capacity = a_other.m_capacity;

        m_data = (T*)calloc(m_capacity, sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_other.m_data[i];
        }
    }
    Array(const T* a_data, uint32_t a_size)
    {
        m_size = a_size;
        m_capacity = a_size;
        if (m_capacity < 1)
        {
            m_capacity = 1;
        }

        m_data = (T*)calloc(m_capacity, sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    ~Array()
    {
        if (m_data != NULL)
        {
            DestroyData();

            free(m_data);
        }
    }

    inline Array& operator =(const Array& a_other)
    {
        if (m_data != NULL)
        {
            DestroyData();

            free(m_data);
        }

        m_size = a_other.m_size;
        m_capacity = a_other.m_capacity;
        m_data = (T*)calloc(m_capacity, sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_other.m_data[i];
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

            memset(m_data, 0, m_capacity * sizeof(T));
        }

        m_size = 0;
    }

    void Push(const T& a_data)
    {
        const uint32_t aSize = m_size + 1;
        IDEFER(m_size = aSize);

        if (aSize > m_capacity)
        {
            const uint32_t cap = m_capacity << 1;
            const uint32_t diff = cap - m_capacity;
            IDEFER(m_capacity = cap);

            m_data = (T*)realloc(m_data, sizeof(T) * cap);
            memset(m_data + m_capacity, 0, diff * sizeof(T));
        }

        m_data[m_size] = a_data;
    }
    void Erase(uint32_t a_index)
    {
        const uint32_t aSize = m_size - 1;
        IDEFER(m_size = aSize);

        if constexpr (std::is_destructible<T>())
        {
            (&(m_data[a_index]))->~T();
        }

        memcpy(m_data + a_index, m_data + a_index + 1, (aSize - a_index) * sizeof(T));
        memset(m_data + m_size, 0, sizeof(T));
    }
    void Reserve(uint32_t a_size)
    {
        IDEFER(m_capacity = a_size);

        if (a_size > m_capacity)
        {
            const uint32_t diff = a_size - m_capacity;

            m_data = (T*)realloc(m_data, sizeof(T) * a_size);
            memset(m_data + m_capacity, 0, diff * sizeof(T));
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
        m_data[a_index] = a_value;
    }
};