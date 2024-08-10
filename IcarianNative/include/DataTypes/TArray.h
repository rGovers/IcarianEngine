#pragma once

#include <cstring>
#include <cstdlib>
#include <vector>

#include "DataTypes/Array.h"
#include "DataTypes/TLockArray.h"

// When in doubt with memory issues write it C style with C++ features
// Using C++ memory features was causing seg-faults and leaks so just done it C style and just manually call the deconstructor when I need to
// All this because I am too lazy to implement constructors
template<typename T>
class TArray
{
private:
    T*             m_data;
    uint32_t       m_size;
    SharedSpinLock m_lock;

    inline void DestroyData()
    {
        if constexpr (!std::is_trivially_destructible<T>())
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
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = a_other.m_size;
        const uint32_t aSize = sizeof(T) * m_size;
        m_data = (T*)malloc(aSize);
        memcpy(m_data, a_other.m_data, aSize);
    }
    TArray(TArray&& a_other)
    {
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = a_other.m_size;
        m_data = a_other.m_data;

        a_other.m_size = 0;
        a_other.m_data = nullptr;
    }
    TArray(const T* a_data, uint32_t a_size)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = a_size;
        m_data = (T*)calloc(m_size, sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    TArray(const T* a_start, const T* a_end)
    {
        const ThreadGuard g = ThreadGuard(m_lock);
        
        const uint32_t aSize = a_end - a_start;

        m_size = aSize / sizeof(T);
        m_data = (T*)calloc(m_size, sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_start[i];
        }
    }
    explicit TArray(const std::vector<T>& a_vec)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

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
        const ThreadGuard g = ThreadGuard(m_lock);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
        }
    }

    TArray& operator =(const TArray& a_other) 
    {
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
        }

        m_size = a_other.m_size;
        m_data = (T*)calloc(m_size, sizeof(T));
        for (uint32_t i = 0; i < m_size; ++i)
        {
            m_data[i] = a_other.m_data[i];
        }

        return *this;
    }

    Array<T> ToArray()
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return Array<T>(m_data, m_size);
    }
    std::vector<T> ToVector() 
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return std::vector<T>(m_data, m_data + m_size);
    }

    TLockArray<T> ToLockArray()
    {
        TLockArray<T> a = TLockArray<T>(m_lock);

        a.SetData(m_data, m_size);

        return a;
    }
    TReadLockArray<T> ToReadLockArray()
    {
        TReadLockArray<T> a = TReadLockArray<T>(m_lock);

        a.SetData(m_data, m_size);

        return a;
    }

    SharedSpinLock& SpinLock()
    {
        return m_lock;
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
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return m_data[a_index];
    }
    inline void LockSet(uint32_t a_index, const T& a_value)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        m_data[a_index] = a_value;
    }

    inline void Push(const T& a_data)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        UPush(a_data);
    }
    void UPush(const T& a_data)
    {
        const uint32_t newSize = m_size + 1;
        m_data = (T*)realloc(m_data, newSize * sizeof(T));
        memset(m_data + m_size, 0, sizeof(T));

        m_data[m_size++] = a_data;
    }

    inline uint32_t PushVal(const T& a_data)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        return UPushVal(a_data);
    }
    uint32_t UPushVal(const T& a_data)
    {
        const uint32_t newSize = m_size + 1;
        m_data = (T*)realloc(m_data, newSize * sizeof(T));
        memset(m_data + m_size, 0, sizeof(T));
        m_data[m_size] = a_data;

        return m_size++;
    }
    inline void PushVals(const T& a_data, uint32_t a_count)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        return UPushVals(a_data, a_count);
    }
    void UPushVals(const T& a_data, uint32_t a_count)
    {
        const uint32_t newSize = m_size + a_count;
        m_data = (T*)realloc(m_data, newSize);
        memset(m_data + m_size, 0, a_count * sizeof(T));

        m_size += a_count;
    }

    T Pop()
    {
        const ThreadGuard g = ThreadGuard(m_lock);
        
        T dat = m_data[--m_size];

        if constexpr (!std::is_trivially_destructible<T>())
        {
            (&(m_data[m_size]))->~T();
        }

        return dat;
    }
    inline void Erase(uint32_t a_index)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        UErase(a_index);
    }
    void UErase(uint32_t a_index)
    {
        const uint32_t newSize = m_size - 1;

        if constexpr (!std::is_trivially_destructible<T>())
        {
            (&(m_data[a_index]))->~T();
        }

        T* dat = (T*)calloc(m_size, sizeof(T));
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, a_index * sizeof(T));
            memcpy(dat + a_index, m_data + (a_index + 1), (m_size - a_index - 1) * sizeof(T));

            free(m_data);
        }

        --m_size;
        m_data = dat;
    }
    void Erase(uint32_t a_start, uint32_t a_end)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        const uint32_t diff = a_end - a_start;

        const uint32_t newSize = m_size - diff;

        if constexpr (!std::is_trivially_destructible<T>())
        {
            for (uint32_t i = a_start; i < a_end; ++i)
            {
                (&(m_data[i]))->~T();
            }
        }

        T* dat = (T*)calloc(newSize, sizeof(T));
        if (m_data != nullptr)
        {
            memcpy(dat, m_data, a_start * sizeof(T));
            memcpy(dat + a_start, m_data + (a_end + 1), (m_size - a_start - diff) * sizeof(T));

            free(m_data);
        }

        m_size -= diff;
        m_data = dat;
    }

    inline void Clear()
    {
        const ThreadGuard g = ThreadGuard(m_lock);

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