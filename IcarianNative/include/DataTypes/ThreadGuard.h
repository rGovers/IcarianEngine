#pragma once

#include "DataTypes/SpinLock.h"

template<typename Lock = SpinLock>
class ThreadGuard
{
private:
    Lock& m_lock;

protected:

public:
    ThreadGuard(Lock& a_lock) : 
        m_lock(a_lock)
    {
        m_lock.Lock();
    }
    ~ThreadGuard()
    {
        m_lock.Unlock();
    }
};

class SharedThreadGuard
{
private:
    SharedSpinLock& m_lock;

protected:

public:
    SharedThreadGuard(SharedSpinLock& a_lock) : 
        m_lock(a_lock)
    {
        m_lock.LockShared();
    }
    ~SharedThreadGuard()
    {
        m_lock.UnlockShared();
    }
};