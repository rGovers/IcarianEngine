#pragma once

#include <cstdint>

#include "Rendering/AnimationController.h"

class RuntimeManager;

class AnimationControllerBindings
{
private:
    AnimationController* m_controller;
    
protected:

public:
    AnimationControllerBindings(AnimationController* a_controller, RuntimeManager* a_runtime);
    ~AnimationControllerBindings();

    uint32_t GenerateAnimatorBuffer() const;
    void DestroyAnimatorBuffer(uint32_t a_addr) const;
    e_AnimationUpdateMode GetAnimatorUpdateMode(uint32_t a_addr) const;
    void SetAnimatorUpdateMode(uint32_t a_addr, e_AnimationUpdateMode a_updateMode) const;
};