#pragma once

#include <cstdint>

#include "DataTypes/TNCArray.h"

class AnimationControllerBindings;
class RuntimeFunction;
class RuntimeManager;

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

class AnimationController
{
private:
    friend class AnimationControllerBindings;

    AnimationControllerBindings*    m_bindings;

    TNCArray<e_AnimationUpdateMode> m_animators;

    RuntimeFunction*                m_updateAnimatorFunc;
    RuntimeFunction*                m_updateAnimatorsFunc;

    AnimationController(RuntimeManager* a_runtime);

protected:

public:
    ~AnimationController();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static std::vector<uint32_t> GetAnimators(e_AnimationUpdateMode a_updateMode);

    static void UpdateAnimator(uint32_t a_index, float a_deltaTime);
    static void UpdateAnimators(e_AnimationUpdateMode a_updateMode, float a_deltaTime);

    static void DispatchUpdate(float a_deltaTime);
};