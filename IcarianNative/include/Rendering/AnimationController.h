// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

#include "DataTypes/TNCArray.h"

class AnimationControllerBindings;
class RuntimeFunction;

enum e_AnimationUpdateMode : uint16_t
{
    AnimationUpdateMode_None = 0,
    // Main and physics thread
    AnimationUpdateMode_Update = 0b1 << 0,
    // Frame thread
    AnimationUpdateMode_FrameUpdate = 0b1 << 1,
    // Thread pool
    AnimationUpdateMode_PooledUpdateLow = 0b1 << 2,
    AnimationUpdateMode_PooledUpdateMedium = 0b1 << 3,
    AnimationUpdateMode_PooledUpdateHigh = 0b1 << 4
};

struct BoneTransformData
{
    glm::mat4 InverseBindPose;
    uint32_t TransformIndex;
};
struct SkeletonData
{
    std::vector<BoneTransformData> BoneData;
};

class AnimationController
{
private:
    friend class AnimationControllerBindings;

    AnimationControllerBindings*    m_bindings;

    TNCArray<e_AnimationUpdateMode> m_animators;
    TNCArray<SkeletonData>          m_skeletons;  

    RuntimeFunction*                m_updateAnimatorFunc;
    RuntimeFunction*                m_updateAnimatorsFunc;

    AnimationController();

protected:

public:
    ~AnimationController();

    static void Init();
    static void Destroy();

    static std::vector<uint32_t> GetAnimators(e_AnimationUpdateMode a_updateMode);

    static void UpdateAnimator(uint32_t a_index, double a_deltaTime);
    static void UpdateAnimators(e_AnimationUpdateMode a_updateMode, double a_deltaTime);

    static void DispatchUpdate(double a_deltaTime);

    static SkeletonData GetSkeleton(uint32_t a_index);
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