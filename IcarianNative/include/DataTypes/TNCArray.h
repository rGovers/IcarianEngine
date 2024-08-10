#pragma once

#include "DataTypes/TLockArray.h"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "DataTypes/Array.h"
#include "DataTypes/ThreadGuard.h"

// Something bout this class causes debuggers to freak out and break on phantom bits of code and language servers to show errors when it works fine?
// Hopefully it is just a bug in the tools or my particular configuration and not the code
// Comments on the line before somehow fix the errors showing up in IDEs

// An array that maintains element indices through erase operations and reuses them
template<typename T, typename StateVal>
class TNCArrayBase
{
private:
    constexpr static uint32_t StateValBitSize = sizeof(StateVal) * 8;

    StateVal*      m_state;
    T*             m_data;
    uint32_t       m_size;

    SharedSpinLock m_lock;

    inline void DestroyData()
    {
        if constexpr (!std::is_trivially_destructible<T>())
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
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = a_other.m_size;
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)calloc(m_size, sizeof(T));
        m_state = (StateVal*)calloc(stateSize, sizeof(StateVal));

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
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);
        
        m_size = a_other.m_size;
        m_state = a_other.m_state;
        m_data = a_other.m_data;

        a_other.m_size = 0;
        a_other.m_state = nullptr;
        a_other.m_data = nullptr;
    }
    TNCArrayBase(const T* a_data, uint32_t a_size)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = a_size;
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)calloc(m_size, sizeof(T));
        m_state = (StateVal*)calloc(stateSize, sizeof(StateVal));

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
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = a_end - a_start;
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)calloc(m_size, sizeof(T));
        m_state = (StateVal*)calloc(stateSize, sizeof(StateVal));


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
        const ThreadGuard g = ThreadGuard(m_lock);

        m_size = (uint32_t)a_vec.size();
        const uint32_t stateSize = m_size / StateValBitSize + 1;

        m_data = (T*)calloc(m_size, sizeof(T));
        m_state = (StateVal*)calloc(stateSize, sizeof(StateVal));

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
        const ThreadGuard g = ThreadGuard(m_lock);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
            free(m_state);
        }
    }

    TNCArrayBase& operator =(const TNCArrayBase& a_other)
    {
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

        if (m_data != nullptr)
        {
            DestroyData();

            free(m_data);
            free(m_state);
        }

        m_size = a_other.m_size;
        const uint32_t stateSize = m_size / StateValBitSize + 1;
        
        m_data = (T*)calloc(m_size, sizeof(T));
        m_state = (StateVal*)calloc(stateSize, sizeof(StateVal));

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

    Array<T> ToArray()
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return Array<T>(m_data, m_size);
    }
    Array<bool> ToStateArray()
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        Array<bool> a;
        a.Reserve(m_size);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            a.Push((m_state[stateIndex] & 0b1 << stateOffset) != 0);
        }

        return a;
    }
    Array<T> ToActiveArray()
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        Array<T> a;
        a.Reserve(m_size);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (m_state[stateIndex] & 0b1 << stateOffset)
            {
                a.Push(m_data[i]);
            }
        }

        return a;
    }

    std::vector<T> ToVector()
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return std::vector<T>(m_data, m_data + m_size);
    }
    std::vector<bool> ToStateVector()
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

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
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

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

    constexpr uint32_t Size() const
    {
        return m_size;
    }
    constexpr bool Empty() const
    {
        return m_size <= 0;
    }

    inline bool Exists(uint32_t a_addr)
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        if constexpr (std::is_pointer<T>())
        {
            if (m_data[a_addr] == nullptr)
            {
                return false;
            }
        }

        const uint32_t stateIndex = a_addr / StateValBitSize;
        const uint32_t stateOffset = a_addr % StateValBitSize;

        // While it works cause ifs just check 0 or non zero making sure it is a 0 or 1 value for correctness as it is used externally
        // Compiler will probably optimize it out anyways
        return (m_state[stateIndex] & 0b1 << stateOffset) != 0;
    }

    inline T& operator [](uint32_t a_index)
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return m_data[a_index];
    }
    void LockSet(uint32_t a_index, const T& a_value)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        const uint32_t stateIndex = a_index / StateValBitSize;
        const uint32_t stateOffset = a_index % StateValBitSize;

        m_state[stateIndex] |= 0b1 << stateOffset;
        m_data[a_index] = a_value;
    }

    void Push(const T& a_data)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

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
            memset(m_state + oldStateSize, 0, sizeof(StateVal));
        }
        else if (m_state == nullptr)
        {
            m_state = (StateVal*)calloc(newStateSize, sizeof(StateVal));
        }

        // Huh sometimes pays to read the docs apparenty realloc is fine with null pointers
        m_data = (T*)realloc(m_data, sizeof(T) * newSize);
        memset(m_data + m_size, 0, sizeof(T));

        const uint32_t stateIndex = m_size / StateValBitSize;
        const uint32_t stateOffset = m_size % StateValBitSize;

        m_state[stateIndex] |= 0b1 << stateOffset;
        m_data[m_size++] = a_data;
    }
    uint32_t PushVal(const T& a_data)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

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
            memset(m_state + oldStateSize, 0, sizeof(StateVal));
        }
        else if (m_state == nullptr) 
        {
            m_state = (StateVal*)calloc(newStateSize, sizeof(StateVal));
        }

        // Huh sometimes pays to read the docs apparenty realloc is fine with null pointers
        m_data = (T*)realloc(m_data, sizeof(T) * newSize);
        memset(m_data + m_size, 0, sizeof(T));

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
        const ThreadGuard g = ThreadGuard(m_lock);

        const uint32_t stateIndex = a_index / StateValBitSize;
        const uint32_t stateOffset = a_index % StateValBitSize;

        if (m_state[stateIndex] & 0b1 << stateOffset)
        {
            m_state[stateIndex] &= ~(0b1 << stateOffset);

            if constexpr (!std::is_trivially_destructible<T>())
            {
                (&(m_data[a_index]))->~T();
            }
            
            memset(&(m_data[a_index]), 0, sizeof(T));
        }        
    }
    void Erase(uint32_t a_start, uint32_t a_end)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        for (uint32_t i = a_start; i < a_end; ++i)
        {
            const uint32_t stateIndex = i / StateValBitSize;
            const uint32_t stateOffset = i % StateValBitSize;

            if (m_state[stateIndex] & 0b1 << stateOffset)
            {
                m_state[stateIndex] &= ~(0b1 << stateOffset);

                if constexpr (!std::is_trivially_destructible<T>())
                {
                    (&(m_data[i]))->~T();
                }
            }
        }

        memset(&(m_data[a_start]), 0, sizeof(T) * (a_end - a_start));
    }

    inline void Clear()
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        DestroyData();

        m_size = 0;
        m_data = nullptr;
    }
};

template<typename T>
using TNCArray = TNCArrayBase<T, uint8_t>;