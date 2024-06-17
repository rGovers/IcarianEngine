#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>

class PhysicsEngine;
class RuntimeFunction;

class IcCharacterListener : public JPH::CharacterContactListener
{
private:
    PhysicsEngine*   m_engine;

    RuntimeFunction* m_onAdjustBody;
    RuntimeFunction* m_onContactValidate;
    RuntimeFunction* m_onContactAdded;
    RuntimeFunction* m_onContactSolve;

protected:

public:
    IcCharacterListener(PhysicsEngine* a_engine);
    virtual ~IcCharacterListener();

    virtual void OnAdjustBodyVelocity(const JPH::CharacterVirtual* a_character, const JPH::Body& a_body, JPH::Vec3& a_velocity, JPH::Vec3& a_angularVelocity);
    virtual bool OnContactValidate(const JPH::CharacterVirtual* a_character, const JPH::Body& a_body, const JPH::SubShapeID& a_shapeId);
    virtual void OnContactAdded(const JPH::CharacterVirtual* a_character, const JPH::BodyID& a_body, const JPH::SubShapeID& a_shapeId, JPH::RVec3Arg a_position, JPH::Vec3Arg a_normal, JPH::CharacterContactSettings& a_settings);
    virtual void OnContactSolve(const JPH::CharacterVirtual* a_character, const JPH::BodyID& a_body, const JPH::SubShapeID& a_shapeId, JPH::RVec3Arg a_position, JPH::Vec3Arg a_normal, JPH::Vec3Arg a_contactVelocity, const JPH::PhysicsMaterial* a_material, JPH::Vec3Arg a_characterVelocity, JPH::Vec3& a_newVelocity);
};