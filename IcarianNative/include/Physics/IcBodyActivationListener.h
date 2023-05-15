#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <vector>

#include "DataTypes/TArray.h"

class IcBodyActivationListener : public JPH::BodyActivationListener
{
private:
    TArray<JPH::BodyID> m_bodies;

protected:

public:
    IcBodyActivationListener();
    virtual ~IcBodyActivationListener();

    std::vector<JPH::BodyID> ToBodies();

    virtual void OnBodyActivated(const JPH::BodyID& a_bodyID, JPH::uint64 a_userData);
    virtual void OnBodyDeactivated(const JPH::BodyID& a_bodyID, JPH::uint64 a_userData);
};