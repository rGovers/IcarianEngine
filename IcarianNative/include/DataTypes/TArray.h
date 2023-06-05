#pragma once

#include <cstring>
#include <cstdlib>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include "DataTypes/TLockArray.h"

// When in doubt with memory issues write it C style with C++ features
// Using C++ memory features was causing seg-faults and leaks so just done it C style and just manually call the deconstructor when I need to
// All this because I am too lazy to implement constructors
template<typename T>
class TArray
{
private:
    std::shared_mutex m_mutex;
    uint32_t          m_size;
    T*                m_data;

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
    constexpr TArray() :
        m_size(0),
        m_data(nullptr) { }
    TArray(const TArray& a_other)
    {
        const std::unique_lock<std::shared_mutex> otherG = std::unique_lock<std::shared_mutex>(a_other.m_mutex);
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_other.m_size;
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memcpy(m_data, a_other.m_data, aSize);
    }
    TArray(TArray&& a_other)
    {
        const std::unique_lock<std::shared_mutex> otherG = std::unique_lock<std::shared_mutex>(a_other.m_mutex);
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_other.m_size;
        m_data = a_other.m_data;
        a_other.m_size = 0;
        a_other.m_data = nullptr;
    }
    TArray(const T* a_data, uint32_t a_size)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_size;
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    TArray(const T* a_start, const T* a_end)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);
        
        const uint32_t aSize = a_end - a_start;
        m_size = aSize / sizeof(T);
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_start[i];
        }
    }
    explicit TArray(const std::vector<T>& a_vec)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = (uint32_t)a_vec.size();
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_vec[i];
        }
    }
    ~TArray()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
        }
    }

    TArray& operator =(const TArray& a_other) 
    {
        const std::unique_lock<std::shared_mutex> otherG = std::unique_lock<std::shared_mutex>(a_other.m_mutex);
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
        }

        m_size = a_other.m_size;
        const uint32_t aSize = m_size * sizeof(T);
        m_data = (T*)malloc(aSize);
        memset(m_data, 0, aSize);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_other.m_data[i];
        }

        return *this;
    }

    std::vector<T> ToVector() 
    {
        const std::shared_lock<std::shared_mutex> g = std::shared_lock<std::shared_mutex>(m_mutex);

        if (m_data == nullptr)
        {
            return std::vector<T>();
        }

        return std::vector<T>(m_data, m_data + m_size);
    }
    TLockArray<T> ToLockArray()
    {
        TLockArray<T> a = TLockArray<T>(m_mutex);

        a.SetData(m_data, m_size);

        return a;
    }
    TReadLockArray<T> ToReadLockArray()
    {
        TReadLockArray<T> a = TReadLockArray<T>(m_mutex);

        a.SetData(m_data, m_size);

        return a;
    }

    inline std::shared_mutex& Lock()
    {
        return m_mutex;
    }
    constexpr uint32_t Size() const
    {
        return m_size;
    }
    constexpr T* Data() const
    {
        return m_data;
    }
    constexpr bool Empty() const
    {
        return m_size <= 0;
    }

    inline T& operator [](uint32_t a_index)
    {
        const std::shared_lock<std::shared_mutex> g = std::shared_lock<std::shared_mutex>(m_mutex);

        return m_data[a_index];
    }
    inline void LockSet(uint32_t a_index, const T& a_value)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_data[a_index] = a_value;
    }

    void Push(const T& a_data)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        const uint32_t aSize = (m_size + 1) * sizeof(T);
        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {   
            memcpy(dat, m_data, m_size * sizeof(T));

            free(m_data);
        }

        dat[m_size++] = a_data;
        
        m_data = dat;
    }
    uint32_t PushVal(const T& a_data)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        const uint32_t aSize = (m_size + 1) * sizeof(T);
        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, m_size * sizeof(T));

            free(m_data);
        }

        dat[m_size] = a_data;

        m_data = dat;

        return m_size++;
    }

    T Pop()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);
        
        T dat = m_data[--m_size];

        if constexpr (std::is_destructible<T>())
        {
            (&(m_data[m_size]))->~T();
        }

        return dat;
    }
    inline void Erase(uint32_t a_index)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        UErase(a_index);
    }
    void UErase(uint32_t a_index)
    {
        constexpr uint32_t Stride = sizeof(T);

        const uint32_t aSize = (m_size - 1) * Stride;

        if constexpr (std::is_destructible<T>())
        {
            (&(m_data[a_index]))->~T();
        }

        const uint32_t offset = a_index * Stride;

        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, offset);
            memcpy(((char*)dat) + offset, ((char*)m_data) + ((a_index + 1) * Stride), (m_size - a_index - 1) * Stride);

            free(m_data);
        }

        --m_size;
        m_data = dat;
    }
    void Erase(uint32_t a_start, uint32_t a_end)
    {
        constexpr uint32_t Stride = sizeof(T);

        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        const uint32_t diff = a_end - a_start;

        const uint32_t aSize = (m_size - diff) * Stride;

        if constexpr (std::is_destructible<T>())
        {
            for (uint32_t i = a_start; i < a_end; ++i)
            {
                (&(m_data[i]))->~T();
            }
        }

        const uint32_t offset = a_start * Stride;

        T* dat = (T*)malloc(aSize);
        memset(dat, 0, aSize);
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, offset);
            memcpy(((char*)dat) + offset, ((char*)m_data) + ((a_end + 1) * Stride), (m_size - a_start - diff) * Stride);

            free(m_data);
        }

        m_size -= diff;
        m_data = dat;
    }

    inline void Clear()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        UClear();
    }
    void UClear()
    {
        DestroyData();

        free(m_data);

        m_size = 0;
        m_data = nullptr;
    }
};