#pragma once

#include <unordered_map>

#include "DataTypes/ThreadGuard.h"

template<typename TKey, typename TValue>
class TUMap
{
private:
    SharedSpinLock                   m_lock;
    std::unordered_map<TKey, TValue> m_map;

protected:

public:
    TUMap()
    {

    }
    TUMap(const TUMap& a_other)
    {
        const ThreadGuard g = ThreadGuard(m_lock);
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        
        m_map = a_other.m_map;
    }
    ~TUMap()
    {

    }

    TUMap& operator =(const TUMap& a_other)
    {
        const ThreadGuard g = ThreadGuard(m_lock);
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);

        m_map = a_other.m_map;

        return *this;
    }

    void Push(const TKey& a_key, const TValue& a_value)
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        const auto iter = m_map.find(a_key);
        if (iter != m_map.end())
        {
            iter->second = a_value;

            return;
        }

        m_map.emplace(a_key, a_value);
    }

    inline bool Exists(const TKey& a_key)
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return m_map.find(a_key) != m_map.end(); 
    }
    inline void Clear()
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        m_map.clear();
    }

    inline TValue& operator [](const TKey& a_key)
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return m_map[a_key];
    }
};