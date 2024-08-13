// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "DataTypes/SpinLock.h"

template<typename T, typename TMutex = SpinLock>
class TLockObj
{
private:
    TMutex* m_lock;
    T       m_obj;

protected:

public:
    explicit TLockObj(const T& a_obj, TMutex* a_mutex)
    {
        m_lock = a_mutex;
        m_lock->Lock();
        
        m_obj = a_obj;
    }
    explicit TLockObj(TMutex* a_mutex)
    {
        m_lock = a_mutex;
        m_lock->Lock();
    }
    ~TLockObj()
    {
        m_lock->Unlock();
    }

    inline T& operator*() 
    {
        return m_obj;
    }
    inline T* operator->() 
    {
        return &m_obj;
    }

    inline T& Get()
    {
        return m_obj;
    }
    inline void Set(const T& a_obj)
    {
        m_obj = a_obj;
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