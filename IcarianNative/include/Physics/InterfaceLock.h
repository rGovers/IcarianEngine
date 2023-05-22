#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/BodyLockInterface.h>

class PhysicsInterfaceReadLock
{
private:
    const JPH::BodyLockInterfaceLocking& m_interface;
    JPH::SharedMutex*                    m_mutex;

protected:

public:
    PhysicsInterfaceReadLock(const JPH::BodyID a_id, const JPH::BodyLockInterfaceLocking& a_interface) : 
        m_interface(a_interface)
    {
        m_mutex = m_interface.LockRead(a_id);
    }
    ~PhysicsInterfaceReadLock()
    {
        m_interface.UnlockRead(m_mutex);
    }
};

class PhysicsInterfaceWriteLock
{
private:
    const JPH::BodyLockInterfaceLocking& m_interface;
    JPH::SharedMutex*                    m_mutex;

protected:

public:
    PhysicsInterfaceWriteLock(const JPH::BodyID a_id, const JPH::BodyLockInterfaceLocking& a_interface) : 
        m_interface(a_interface)
    {
        m_mutex = m_interface.LockWrite(a_id);
    }
    ~PhysicsInterfaceWriteLock()
    {
        m_interface.UnlockWrite(m_mutex);
    }
};