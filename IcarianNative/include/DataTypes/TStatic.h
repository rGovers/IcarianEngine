// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <thread>
#include <unordered_map>

#include "DataTypes/ThreadGuard.h"

template<typename T>
class TStatic
{
private:
    SharedSpinLock                          m_lock;
    std::unordered_map<std::thread::id, T*> m_data;

protected:

public:
    TStatic()
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        m_data = std::unordered_map<std::thread::id, T*>();
    }
    TStatic(const TStatic& a_other)
    {
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

        m_data = a_other.m_data;
    }
    ~TStatic()
    {
        Clear();
    }

    TStatic& operator =(const TStatic& a_other)
    {
        const ThreadGuard otherG = ThreadGuard(a_other.m_lock);
        const ThreadGuard g = ThreadGuard(m_lock);

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

        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return *(m_data[id]);
    }
    inline T* operator->() 
    {
        const std::thread::id id = std::this_thread::get_id();

        const SharedThreadGuard g = SharedThreadGuard(m_lock);

        return m_data[id];
    }

    T& Push(const T& a_data)
    {
        const std::thread::id id = std::this_thread::get_id();

        T* d = new T(a_data);
        
        const ThreadGuard g = ThreadGuard(m_lock);

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

        const SharedThreadGuard g = SharedThreadGuard(m_lock);

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

        const ThreadGuard g = ThreadGuard(m_lock);

        const auto iter = m_data.find(id);
        if (iter != m_data.end())
        {
            delete iter->second;

            m_data.erase(iter);
        }
    }

    void Clear()
    {
        const ThreadGuard g = ThreadGuard(m_lock);

        for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
        {
            delete iter->second;
        }

        m_data.clear();
    }
};

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