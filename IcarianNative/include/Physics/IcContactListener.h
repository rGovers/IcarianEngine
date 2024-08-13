// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/ContactListener.h>

class PhysicsEngine;
class RuntimeFunction;

#include "EnginePhysicsBodyInteropStructures.h"

class IcContactListener : public JPH::ContactListener
{
private:
    PhysicsEngine*   m_engine;
    
    RuntimeFunction* m_onCollisionEnterFunc;
    RuntimeFunction* m_onCollisionStayFunc;
    RuntimeFunction* m_onCollisionExitFunc;

protected:

public:
    IcContactListener(PhysicsEngine* a_engine);
    virtual ~IcContactListener();

    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& a_lhs, const JPH::Body& a_rhs, JPH::RVec3Arg a_baseOffset, const JPH::CollideShapeResult& a_collisionResult);
    virtual void OnContactAdded(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings);
    virtual void OnContactPersisted(const JPH::Body& a_lhs, const JPH::Body& a_rhs, const JPH::ContactManifold& a_manifold, JPH::ContactSettings& a_ioSettings);
    virtual void OnContactRemoved(const JPH::SubShapeIDPair& a_shapePair);
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