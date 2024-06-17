#include "Physics/IcCharacterListener.h"

#include "Physics/PhysicsEngine.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"

IcCharacterListener::IcCharacterListener(PhysicsEngine* a_engine)
{
    m_engine = a_engine;

    m_onAdjustBody = RuntimeManager::GetFunction("IcarianEngine.Physics", "CharacterController", ":OnAdjustVelocity");
    m_onContactValidate = RuntimeManager::GetFunction("IcarianEngine.Physics", "CharacterController", ":OnContactValidate");
    m_onContactAdded = RuntimeManager::GetFunction("IcarianEngine.Physics", "CharacterController", ":OnContactAdd");
    m_onContactSolve = RuntimeManager::GetFunction("IcarianEngine.Physics", "CharacterController", ":OnContactSolve");
}
IcCharacterListener::~IcCharacterListener()
{
    delete m_onAdjustBody;
    delete m_onContactValidate;
    delete m_onContactAdded;
    delete m_onContactSolve;
}

void IcCharacterListener::OnAdjustBodyVelocity(const JPH::CharacterVirtual* a_character, const JPH::Body& a_body, JPH::Vec3& a_velocity, JPH::Vec3& a_angularVelocity)
{
    uint32_t addr = (uint32_t)(a_character->GetUserData() >> 32);

    const JPH::BodyID id = a_body.GetID();
    uint32_t bodyAddr = m_engine->GetBodyAddr(id.GetIndex());

    glm::vec3 vel = glm::vec3(a_velocity.GetX(), a_velocity.GetY(), a_velocity.GetZ());
    glm::vec3 angularVel = glm::vec3(a_angularVelocity.GetX(), a_angularVelocity.GetY(), a_angularVelocity.GetZ());

    glm::vec3* velPtr = &vel;
    glm::vec3* angularVelPtr = &angularVel;

    void* args[] =
    {
        &addr,
        &bodyAddr,
        &velPtr,
        &angularVelPtr
    };

    m_onAdjustBody->Exec(args);

    a_velocity = JPH::Vec3(vel.x, vel.y, vel.z);
    a_angularVelocity = JPH::Vec3(angularVel.x, angularVel.y, angularVel.z);
}
bool IcCharacterListener::OnContactValidate(const JPH::CharacterVirtual* a_character, const JPH::Body& a_body, const JPH::SubShapeID& a_shapeId)
{
    uint32_t addr = (uint32_t)(a_character->GetUserData() >> 32);

    const JPH::BodyID id = a_body.GetID();
    uint32_t bodyAddr = m_engine->GetBodyAddr(id.GetIndex());

    int32_t state = 1;
    int32_t* statePtr = &state;

    void* args[] =
    {
        &addr,
        &bodyAddr,
        &statePtr   
    };

    m_onContactValidate->Exec(args);

    return state != 0;
}
void IcCharacterListener::OnContactAdded(const JPH::CharacterVirtual* a_character, const JPH::BodyID& a_body, const JPH::SubShapeID& a_shapeId, JPH::RVec3Arg a_position, JPH::Vec3Arg a_normal, JPH::CharacterContactSettings& a_settings)
{
    uint32_t addr = (uint32_t)(a_character->GetUserData() >> 32);
    uint32_t bodyAddr = m_engine->GetBodyAddr(a_body.GetIndex());

    glm::vec3 pos = glm::vec3(a_position.GetX(), a_position.GetY(), a_position.GetZ());
    glm::vec3 norm = glm::vec3(a_normal.GetX(), a_normal.GetY(), a_normal.GetZ());

    void* args[] =
    {
        &addr,
        &bodyAddr,
        &pos,
        &norm
    };

    m_onContactAdded->Exec(args);
}
void IcCharacterListener::OnContactSolve(const JPH::CharacterVirtual* a_character, const JPH::BodyID& a_body, const JPH::SubShapeID& a_shapeId, JPH::RVec3Arg a_position, JPH::Vec3Arg a_normal, JPH::Vec3Arg a_contactVelocity, const JPH::PhysicsMaterial* a_material, JPH::Vec3Arg a_characterVelocity, JPH::Vec3& a_newVelocity)
{
    uint32_t addr = (uint32_t)(a_character->GetUserData() >> 32);
    uint32_t bodyAddr = m_engine->GetBodyAddr(a_body.GetIndex());

    glm::vec3 pos = glm::vec3(a_position.GetX(), a_position.GetY(), a_position.GetZ());
    glm::vec3 norm = glm::vec3(a_normal.GetX(), a_normal.GetY(), a_normal.GetZ());
    glm::vec3 contactVelocity = glm::vec3(a_contactVelocity.GetX(), a_contactVelocity.GetY(), a_contactVelocity.GetZ());
    glm::vec3 characterVelocity = glm::vec3(a_characterVelocity.GetX(), a_characterVelocity.GetY(), a_characterVelocity.GetZ());

    glm::vec3 outVel = glm::vec3(a_newVelocity.GetX(), a_newVelocity.GetY(), a_newVelocity.GetZ());
    glm::vec3* outVelPtr = &outVel;

    void* args[] =
    {
        &addr,
        &bodyAddr,
        &pos,
        &norm,
        &contactVelocity,
        &characterVelocity,
        &outVelPtr
    };

    m_onContactSolve->Exec(args);

    a_newVelocity = JPH::Vec3(outVel.x, outVel.y, outVel.z);
}