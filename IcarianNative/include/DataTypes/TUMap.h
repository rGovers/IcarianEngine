#pragma once

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

template<typename TKey, typename TValue>
class TUMap
{
private:
    std::shared_mutex                m_mutex;
    std::unordered_map<TKey, TValue> m_map;

protected:

public:
    TUMap()
    {

    }
    TUMap(const TUMap& a_other)
    {
        const std::unique_lock g = std::unique_lock(m_mutex);
        const std::unique_lock otherG = std::unique_lock(a_other.m_mutex);
        
        m_map = a_other.m_map;
    }
    ~TUMap()
    {

    }

    TUMap& operator =(const TUMap& a_other)
    {
        const std::unique_lock g = std::unique_lock(m_mutex);
        const std::unique_lock otherG = std::unique_lock(a_other.m_mutex);

        m_map = a_other.m_map;

        return *this;
    }

    void Push(const TKey& a_key, const TValue& a_value)
    {
        const std::unique_lock g = std::unique_lock(m_mutex);

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
        const std::shared_lock g = std::shared_lock(m_mutex);

        return m_map.find(a_key) != m_map.end(); 
    }
    inline void Clear()
    {
        const std::unique_lock g = std::unique_lock(m_mutex);

        m_map.clear();
    }

    inline TValue& operator [](const TKey& a_key)
    {
        const std::shared_lock g = std::shared_lock(m_mutex);

        return m_map[a_key];
    }
};