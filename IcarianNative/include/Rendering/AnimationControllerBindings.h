#pragma once

#include <cstdint>

#include "Rendering/AnimationController.h"

class AnimationControllerBindings
{
private:
    AnimationController* m_controller;
    
protected:

public:
    AnimationControllerBindings(AnimationController* a_controller);
    ~AnimationControllerBindings();

    uint32_t GenerateAnimatorBuffer() const;
    void DestroyAnimatorBuffer(uint32_t a_addr) const;
    e_AnimationUpdateMode GetAnimatorUpdateMode(uint32_t a_addr) const;
    void SetAnimatorUpdateMode(uint32_t a_addr, e_AnimationUpdateMode a_updateMode) const;

    uint32_t CreateSkeletonBuffer() const;
    void DestroySkeletonBuffer(uint32_t a_addr) const;
    void ClearSkeletonBuffer(uint32_t a_addr) const;
    void PushSkeletonBoneData(uint32_t a_addr, uint32_t a_transformIndex, const glm::mat4& a_inverseBindPose) const;
};