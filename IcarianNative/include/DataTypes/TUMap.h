// Icarian Engine - C# Game Engine
// 
// License at end of file.

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