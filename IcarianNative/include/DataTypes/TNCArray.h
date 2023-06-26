#pragma once

#include "DataTypes/TLockArray.h"
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <vector>

// Something bout this class causes debuggers to freak out and break on phantom bits of code and language servers to show errors when it works fine?
// Hopefully it is just a bug in the tools or my particular configuration and not the code
// Comments on the line before somehow fix the errors showing up in IDEs

// An array that maintains element indices through erase operations and reuses them
template<typename T, typename StateVal>
class TNCArrayBase
{
private:
    constexpr static uint32_t StateValBitSize = sizeof(StateVal) * 8;

    uint32_t  m_size;
    StateVal* m_state;
    T*        m_data;

    std::shared_mutex m_mutex;

    inline void DestroyData()
    {
        if constexpr (std::is_destructible<T>())
        {
            for (uint32_t i = 0; i < m_size; ++i)
            {
                const uint32_t stateIndex = i / StateValBitSize;
                const uint32_t stateOffset = i % StateValBitSize;

                if (m_state[stateIndex] & 0b1 << stateOffset)
                {
                    (&(m_data[i]))->~T();
                }
            }

            memset(m_data, 0, sizeof(T) * m_size);
        }
    }

protected:

public:
    constexpr TNCArrayBase() : 
        m_size(0),
        m_state(nullptr),
        m_data(nullptr) { }
    TNCArrayBase(const TNCArrayBase& a_other)
    {
        const std::unique_lock<std::shared_mutex> otherG = std::unique_lock<std::shared_mutex>(a_other.m_mutex);
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_other.m_size;
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)malloc(sizeof(T) * m_size);
        m_state = (StateVal*)malloc(sizeof(StateVal) * stateSize);

        memset(m_data, 0, sizeof(T) * m_size);
        memset(m_state, 0, sizeof(StateVal) * stateSize);

        for (uint32_t i = 0; i < stateSize; ++i)
        {
            m_state[i] = a_other.m_state[i];
        }

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (m_state[stateIndex] & 0b1 << stateOffset)
            {
                m_data[i] = a_other.m_data[i];
            }
        }
    }
    TNCArrayBase(TNCArrayBase&& a_other)
    {
        const std::unique_lock<std::shared_mutex> otherG = std::unique_lock<std::shared_mutex>(a_other.m_mutex);
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_other.m_size;
        m_state = a_other.m_state;
        m_data = a_other.m_data;
        a_other.m_size = 0;
        a_other.m_state = nullptr;
        a_other.m_data = nullptr;
    }
    TNCArrayBase(const T* a_data, uint32_t a_size)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_size;
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)malloc(sizeof(T) * m_size);
        m_state = (StateVal*)malloc(sizeof(StateVal) * stateSize);

        memset(m_data, 0, sizeof(T) * m_size);
        memset(m_state, 0, sizeof(StateVal) * stateSize); 

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            m_state[stateIndex] |= 0b1 << stateOffset;
            m_data[i] = a_data[i];
        }
    }
    TNCArrayBase(const T* a_start, const T* a_end)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = a_end - a_start;
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)malloc(sizeof(T) * m_size);
        m_state = (StateVal*)malloc(sizeof(StateVal) * stateSize);

        memset(m_data, 0, sizeof(T) * m_size);
        memset(m_state, 0, sizeof(StateVal) * stateSize);

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            m_state[stateIndex] |= 0b1 << stateOffset;
            m_data[i] = a_start[i];
        }
    }
    explicit TNCArrayBase(const std::vector<T>& a_vec)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        m_size = (uint32_t)a_vec.size();
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)malloc(sizeof(T) * m_size);
        m_state = (StateVal*)malloc(sizeof(StateVal) * stateSize);

        memset(m_data, 0, sizeof(T) * m_size);
        memset(m_state, 0, sizeof(StateVal) * stateSize);

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            m_state[stateIndex] |= 0b1 << stateOffset;
            m_data[i] = a_vec[i];
        }
    }
    ~TNCArrayBase()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
            free(m_state);
        }
    }

    TNCArrayBase& operator =(const TNCArrayBase& a_other)
    {
        const std::unique_lock<std::shared_mutex> otherG = std::unique_lock<std::shared_mutex>(a_other.m_mutex);
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
            free(m_state);
        }

        m_size = a_other.m_size;
        const uint32_t stateSize = m_size / StateValBitSize + 1;
        
        m_data = (T*)malloc(sizeof(T) * m_size);
        m_state = (StateVal*)malloc(sizeof(StateVal) * stateSize);

        memset(m_data, 0, sizeof(T) * m_size);
        memset(m_state, 0, sizeof(StateVal) * stateSize);

        for (uint32_t i = 0; i < stateSize; ++i)
        {
            m_state[i] = a_other.m_state[i];
        }

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (m_state[stateIndex] & 0b1 << stateOffset)
            {
                m_data[i] = a_other.m_data[i];
            }
        }

        return *this;
    }

    std::vector<T> ToVector()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        std::vector<T> vec;
        vec.reserve(m_size);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            vec.push_back(m_data[i]);
        }

        return vec;
    }
    std::vector<bool> ToStateVector()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        std::vector<bool> vec;
        vec.reserve(m_size);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            vec.push_back((m_state[stateIndex] & 0b1 << stateOffset) != 0);
        }

        return vec;
    }
    std::vector<T> ToActiveVector()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        std::vector<T> vec;
        vec.reserve(m_size);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (m_state[stateIndex] & 0b1 << stateOffset)
            {
                vec.push_back(m_data[i]);
            }
        }

        return vec;
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

    constexpr uint32_t Size() const
    {
        return m_size;
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
    void LockSet(uint32_t a_index, const T& a_value)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        const uint32_t stateIndex = a_index / StateValBitSize;
        const uint32_t stateOffset = a_index % StateValBitSize;

        m_state[stateIndex] |= 0b1 << stateOffset;
        m_data[a_index] = a_value;
    }

    void Push(const T& a_data)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (!(m_state[stateIndex] & 0b1 << stateOffset))
            {
                m_state[stateIndex] |= 0b1 << stateOffset;
                m_data[i] = a_data;

                return;
            }
        }

        const uint32_t newSize = m_size + 1;

        const uint32_t oldStateSize = m_size / StateValBitSize + 1;
        const uint32_t newStateSize = newSize / StateValBitSize + 1;

        if (oldStateSize != newStateSize)
        {
            m_state = (StateVal*)realloc(m_state, sizeof(StateVal) * newStateSize);
            memset((char*)m_state + oldStateSize * sizeof(StateVal), 0, sizeof(StateVal));
        }
        else if (m_state == nullptr)
        {
            m_state = (StateVal*)malloc(sizeof(StateVal) * newStateSize);
            memset(m_state, 0, sizeof(StateVal) * newStateSize);
        }

        // Huh sometimes pays to read the docs apparenty realloc is fine with null pointers
        m_data = (T*)realloc(m_data, sizeof(T) * newSize);
        memset((char*)m_data + m_size * sizeof(T), 0, sizeof(T));

        const uint32_t stateIndex = m_size / StateValBitSize;
        const uint32_t stateOffset = m_size % StateValBitSize;

        m_state[stateIndex] |= 0b1 << stateOffset;
        m_data[m_size++] = a_data;
    }
    uint32_t PushVal(const T& a_data)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (!(m_state[stateIndex] & 0b1 << stateOffset))
            {
                m_state[stateIndex] |= 0b1 << stateOffset;
                m_data[i] = a_data;

                return i;
            }
        }

        const uint32_t newSize = m_size + 1;

        const uint32_t oldStateSize = m_size / StateValBitSize + 1;
        const uint32_t newStateSize = newSize / StateValBitSize + 1;

        if (oldStateSize != newStateSize)
        {
            m_state = (StateVal*)realloc(m_state, sizeof(StateVal) * newStateSize);
            memset(((char*)m_state) + oldStateSize * sizeof(StateVal), 0, sizeof(StateVal));
        }
        else if (m_state == nullptr) 
        {
            m_state = (StateVal*)malloc(sizeof(StateVal) * newStateSize);
            memset(m_state, 0, sizeof(StateVal) * newStateSize);
        }

        // Huh sometimes pays to read the docs apparenty realloc is fine with null pointers
        m_data = (T*)realloc(m_data, sizeof(T) * newSize);
        memset((char*)m_data + m_size * sizeof(T), 0, sizeof(T));

        const uint32_t stateIndex = m_size / StateValBitSize;
        const uint32_t stateOffset = m_size % StateValBitSize;

        m_state[stateIndex] |= 0b1 << stateOffset;
        m_data[m_size] = a_data;

        return m_size++;
    }

    template<typename... Args>
    void CPush(Args&&... args)
    {
        Push(T(std::forward<Args>(args)...));
    }
    template<typename... Args>
    uint32_t CPushVal(Args&&... args)
    {
        return PushVal(T(std::forward<Args>(args)...));
    }

    void Erase(uint32_t a_index)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        const uint32_t stateIndex = a_index / StateValBitSize;
        const uint32_t stateOffset = a_index % StateValBitSize;

        if (m_state[stateIndex] & 0b1 << stateOffset)
        {
            m_state[stateIndex] &= ~(0b1 << stateOffset);

            if constexpr (std::is_destructible<T>())
            {
                (&(m_data[a_index]))->~T();
            }
            
            memset(&(m_data[a_index]), 0, sizeof(T));
        }        
    }
    void Erase(uint32_t a_start, uint32_t a_end)
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        for (uint32_t i = a_start; i < a_end; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (m_state[stateIndex] & 0b1 << stateOffset)
            {
                m_state[stateIndex] &= ~(0b1 << stateOffset);

                if constexpr (std::is_destructible<T>())
                {
                    (&(m_data[i]))->~T();
                }
            }
        }

        memset(&(m_data[a_start]), 0, sizeof(T) * (a_end - a_start));
    }

    inline void Clear()
    {
        const std::unique_lock<std::shared_mutex> g = std::unique_lock<std::shared_mutex>(m_mutex);

        DestroyData();

        m_size = 0;
        m_data = nullptr;
    }
};

template<typename T>
using TNCArray = TNCArrayBase<T, uint8_t>;