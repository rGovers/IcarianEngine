// Icarian Engine - C# Game Engine
// 
// License at end of file.

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