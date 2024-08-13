// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Physics/IcBroadPhaseLayerInterface.h"

#include "Core/IcarianAssert.h"
#include "IcarianError.h"
#include "Physics/PhysicsEngine.h"

IcBroadPhaseLayerInterface::IcBroadPhaseLayerInterface()
{

}
IcBroadPhaseLayerInterface::~IcBroadPhaseLayerInterface()
{

}

JPH::uint IcBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
    return 3;
}
JPH::BroadPhaseLayer IcBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer a_layer) const
{
    IVERIFY(a_layer < 8);
    
    switch (a_layer)
    {
    case PhysicsEngine::LayerNonMoving:
    {
        return PhysicsEngine::BroadphaseNonMoving;
    }
    case PhysicsEngine::LayerTrigger:
    {
        return PhysicsEngine::BroadphaseTrigger;
    }
    }

    return PhysicsEngine::BroadphaseMoving;
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char* IcBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer a_layer) const
{
    switch ((JPH::BroadPhaseLayer::Type)a_layer)
    {
    case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseNonMoving:
    {
        return "NON_MOVING";
    }
    case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseMoving:
    {
        return "MOVING";
    }
    case (JPH::BroadPhaseLayer::Type)PhysicsEngine::BroadphaseTrigger:
    {
        return "TRIGGER";
    }
    }

    ICARIAN_ASSERT(0);
}
#endif 

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