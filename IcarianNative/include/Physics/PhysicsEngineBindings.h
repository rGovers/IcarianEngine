// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <Jolt/Core/Core.h>

class PhysicsEngine;

#include "EnginePhysicsInteropStructures.h"
#include "EngineRigidBodyInteropStructures.h"

class PhysicsEngineBindings
{
private:
    PhysicsEngine* m_engine;

    void AddBody(JPH::uint32 a_id, uint32_t a_index) const;
    
protected:

public:
    PhysicsEngineBindings(PhysicsEngine* a_engine);
    ~PhysicsEngineBindings();

    uint32_t CreateSphereShape(float a_radius) const;
    float GetSphereShapeRadius(uint32_t a_addr) const;

    uint32_t CreateBoxShape(const glm::vec3& a_extents) const;
    glm::vec3 GetBoxShapeExtents(uint32_t a_addr) const;

    uint32_t CreateCapsuleShape(float a_height, float a_radius) const;
    float GetCapsuleShapeHeight(uint32_t a_addr) const;
    float GetCasuleShapeRadius(uint32_t a_addr) const;

    uint32_t CreateCylinderShape(float a_height, float a_radius) const;
    float GetCylinderShapeHeight(uint32_t a_addr) const;
    float GetCylinderShapeRadius(uint32_t a_addr) const;

    uint32_t CreateMeshShape(const std::filesystem::path& a_path) const;

    void DestroyCollisionShape(uint32_t a_addr) const;

    uint32_t CreateCharacterController(uint32_t a_transformAddr, uint32_t a_colliderAddr, const glm::vec3& a_up, float a_slopeAngle, float a_mass) const;
    void DestroyCharacterController(uint32_t a_addr) const;
    glm::vec3 GetCharacterControllerVelocity(uint32_t a_addr) const;
    void SetCharacterControllerVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const;

    uint32_t CreatePhysicsBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const;
    void DestroyPhysicsBody(uint32_t a_addr) const;

    void SetPhysicsBodyPosition(uint32_t a_addr, const glm::vec3& a_pos) const;
    glm::vec3 GetPhysicsBodyPosition(uint32_t a_addr) const;
    void SetPhysicsBodyRotation(uint32_t a_addr, const glm::quat& a_rot) const;
    glm::quat GetPhysicsBodyRotation(uint32_t a_addr) const;

    uint32_t CreateRigidBody(uint32_t a_transformAddr, uint32_t a_colliderAddr, uint32_t a_layer, float a_mass) const;
    glm::vec3 GetRigidBodyVelocity(uint32_t a_addr) const;

    void SetRigidBodyGravityFactor(uint32_t a_addr, float a_factor) const;
    float GetRigidBodyGravityFactor(uint32_t a_addr) const;
    void SetRigidBodyVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const;
    glm::vec3 GetRigidBodyAngularVelocity(uint32_t a_addr) const;
    void SetRigidBodyAngularVelocity(uint32_t a_addr, const glm::vec3& a_velocity) const;
    void RigidBodyAddForce(uint32_t a_addr, const glm::vec3& a_force, e_ForceMode a_mode) const;
    void RigidBodyAddTorque(uint32_t a_addr, const glm::vec3& a_torque, e_ForceMode a_mode) const;

    uint32_t CreateTriggerBody(uint32_t a_transformAddr, uint32_t a_colliderAddr) const;

    void SetGravity(const glm::vec3& a_gravity) const;
    glm::vec3 GetGravity() const;

    bool GetObjectLayerCollision(uint32_t a_lhs, uint32_t a_rhs) const;
    void SetObjectLayerCollision(uint32_t a_lhs, uint32_t a_rhs, bool a_state) const;

    RaycastResultBuffer* Raycast(const glm::vec3& a_pos, const glm::vec3& a_dir, uint32_t* a_resultCount) const;
    uint32_t* SphereCollision(const glm::vec3& a_pos, float a_radius, uint32_t* a_resultCount) const;
    uint32_t* BoxCollision(const glm::mat4& a_transform, const glm::vec3& a_extents, uint32_t* a_resultCount) const;
    uint32_t* AABBCollision(const glm::vec3& a_min, const glm::vec3& a_max, uint32_t* a_resultCount) const;
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