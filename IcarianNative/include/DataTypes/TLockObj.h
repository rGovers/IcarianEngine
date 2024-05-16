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