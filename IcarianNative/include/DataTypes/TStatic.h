#pragma once

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

template<typename T>
class TStatic
{
private:
    std::shared_mutex                       m_mutex;
    std::unordered_map<std::thread::id, T*> m_data;

protected:

public:
    TStatic()
    {
        m_data = std::unordered_map<std::thread::id, T*>();
    }
    TStatic(const TStatic& a_other)
    {
        const std::unique_lock otherG = std::unique_lock(a_other.m_mutex);
        const std::unique_lock g = std::unique_lock(m_mutex);

        m_data = a_other.m_data;
    }
    ~TStatic()
    {
        Clear();
    }

    TStatic& operator =(const TStatic& a_other)
    {
        const std::unique_lock otherG = std::unique_lock(a_other.m_mutex);
        const std::unique_lock g = std::unique_lock(m_mutex);

        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            if (iter->second != nullptr)
            {
                delete iter->second;
            }
        }

        m_data = a_other.m_data;
    }

    inline T& operator*() 
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::shared_lock g = std::shared_lock(m_mutex);
        return *(m_data[id]);
    }
    inline T* operator->() 
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::shared_lock g = std::shared_lock(m_mutex);
        return m_data[id];
    }

    T& Push(const T& a_data)
    {
        const std::thread::id id = std::this_thread::get_id();

        T* d = new T(a_data);
        
        const std::unique_lock g = std::unique_lock(m_mutex);

        auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            delete iter->second;
            iter->second = d;
        }
        else
        {
            m_data.emplace(id, d);
        }

        return *d;
    }

    inline bool Exists()
    {
        return Get() != nullptr;
    }

    T* Get()
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::shared_lock g = std::shared_lock(m_mutex);
        const auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            return iter->second;
        }

        return nullptr;
    }

    void Erase()
    {
        const std::thread::id id = std::this_thread::get_id();

        const std::unique_lock g = std::unique_lock(m_mutex);
        auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            delete iter->second;

            m_data.erase(iter);
        }
    }

    void Clear()
    {
        const std::unique_lock g = std::unique_lock(m_mutex);

        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            delete iter->second;
        }

        m_data.clear();
    }
};