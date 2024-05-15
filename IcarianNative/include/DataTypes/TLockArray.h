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