#include "Physics/IcBodyActivationListener.h"

#include <Jolt/Physics/Body/BodyID.h>
#include <vector>

#include "DataTypes/TLockArray.h"

IcBodyActivationListener::IcBodyActivationListener()
{

}
IcBodyActivationListener::~IcBodyActivationListener()
{

}

std::vector<JPH::BodyID> IcBodyActivationListener::ToBodies()
{
    return m_bodies.ToVector();
}

void IcBodyActivationListener::OnBodyActivated(const JPH::BodyID& a_bodyID, JPH::uint64 a_userData)
{
    {
        TLockArray<JPH::BodyID> a = m_bodies.ToLockArray();
        for (JPH::BodyID& val : a)
        {
            if (val.IsInvalid())
            {
                val = a_bodyID;

                return;
            }
        }
    }

    m_bodies.Push(a_bodyID);
}
void IcBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& a_bodyID, JPH::uint64 a_userData)
{
    TLockArray<JPH::BodyID> a = m_bodies.ToLockArray();
    for (JPH::BodyID& val : a) 
    {
        if (val == a_bodyID) 
        {
            val = JPH::BodyID();

            return;
        }
    }
}