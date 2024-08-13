// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/AnimationController.h"

#include "Rendering/AnimationControllerBindings.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "ThreadJob.h"
#include "ThreadPool.h"
#include "Trace.h"

static AnimationController* Instance = nullptr;

AnimationController::AnimationController()
{
    TRACE("Initializing AnimationController");
    m_updateAnimatorFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering.Animation", "Animator", ":UpdateAnimatorS(uint,double)");
    m_updateAnimatorsFunc = RuntimeManager::GetFunction("IcarianEngine.Rendering.Animation", "Animator", ":UpdateAnimatorsS(uint[],double)");

    m_bindings = new AnimationControllerBindings(this);
}
AnimationController::~AnimationController()
{
    delete m_bindings;

    delete m_updateAnimatorFunc;
    delete m_updateAnimatorsFunc;
}

void AnimationController::Init()
{
    if (Instance == nullptr)
    {
        Instance = new AnimationController();
    }
}
void AnimationController::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

std::vector<uint32_t> AnimationController::GetAnimators(e_AnimationUpdateMode a_updateMode)
{   
    const std::vector<bool> stateVector = Instance->m_animators.ToStateVector();
    const std::vector<e_AnimationUpdateMode> modeVector = Instance->m_animators.ToVector();

    const uint32_t size = (uint32_t)stateVector.size();
    
    std::vector<uint32_t> animators;
    animators.reserve(size);

    for (uint32_t i = 0; i < size; ++i)
    {
        if (stateVector[i] && (modeVector[i] & a_updateMode))
        {
            animators.push_back(i);
        }
    }

    return animators;
}

void AnimationController::UpdateAnimator(uint32_t a_index, double a_deltaTime)
{
    void* args[] = 
    { 
        &a_index, 
        &a_deltaTime 
    };

    Instance->m_updateAnimatorFunc->Exec(args);
}
void AnimationController::UpdateAnimators(e_AnimationUpdateMode a_updateMode, double a_deltaTime)
{
    const std::vector<uint32_t> animators = GetAnimators(a_updateMode);

    const uint32_t count = (uint32_t)animators.size();
    if (count > 0)
    {
        MonoArray* animatorsArray = mono_array_new(mono_domain_get(), mono_get_uint32_class(), (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            mono_array_set(animatorsArray, uint32_t, i, animators[i]);
        }

        void* args[] = 
        { 
            animatorsArray, 
            &a_deltaTime 
        };

        Instance->m_updateAnimatorsFunc->Exec(args);
    }
}

class AnimatorThreadJob : public ThreadJob
{
private:
    uint32_t m_animator;
    double   m_deltaTime;

protected:

public:
    constexpr AnimatorThreadJob(uint32_t a_animator, double a_deltaTime, e_JobPriority a_priority) : ThreadJob(a_priority),
        m_animator(a_animator),
        m_deltaTime(a_deltaTime)
    {
        
    }

    inline void Execute()
    {
        AnimationController::UpdateAnimator(m_animator, m_deltaTime);
    }
};

void AnimationController::DispatchUpdate(double a_deltaTime)
{
    const std::vector<bool> stateVector = Instance->m_animators.ToStateVector();
    const std::vector<e_AnimationUpdateMode> modeVector = Instance->m_animators.ToVector();

    const uint32_t size = (uint32_t)stateVector.size();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (stateVector[i])
        {
            switch (modeVector[i])
            {
            case AnimationUpdateMode_PooledUpdateLow:
            {
                ThreadPool::PushJob(new AnimatorThreadJob(i, a_deltaTime, JobPriority_EngineLow));

                break;
            }
            case AnimationUpdateMode_PooledUpdateMedium:
            {
                ThreadPool::PushJob(new AnimatorThreadJob(i, a_deltaTime, JobPriority_EngineMedium));

                break;
            }
            case AnimationUpdateMode_PooledUpdateHigh:
            {
                ThreadPool::PushJob(new AnimatorThreadJob(i, a_deltaTime, JobPriority_EngineHigh));

                break;
            }
            default:
            {
                continue;
            }
            }
        }
    }
}

SkeletonData AnimationController::GetSkeleton(uint32_t a_index)
{
    return Instance->m_skeletons[a_index];
}

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