#include "Rendering/AnimationControllerBindings.h"

#include "Flare/ColladaLoader.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/AnimationController.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static AnimationControllerBindings* Instance = nullptr;

struct RuntimeBoneData
{
    MonoArray* Names;
    MonoArray* Parents;
    MonoArray* BindPoses;
};

struct RuntimeAnimationFrame
{
    float Time;
    MonoArray* Transform;
};
struct RuntimeAnimationData
{
    MonoString* Name;
    MonoArray* Frames;
};

#define ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GenerateBuffer, { return Instance->GenerateAnimatorBuffer(); }) \
    F(void, IcarianEngine.Rendering.Animation, Animator, DestroyBuffer, { Instance->DestroyAnimatorBuffer(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GetUpdateMode, { return (uint32_t)Instance->GetAnimatorUpdateMode(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, Animator, SetUpdateMode, { Instance->SetAnimatorUpdateMode(a_addr, (e_AnimationUpdateMode)a_updateMode); }, uint32_t a_addr, uint32_t a_updateMode) \
    \
    F(MonoArray*, IcarianEngine.Rendering.Animation, AnimationClip, LoadColladaAnimation, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return Instance->LoadAnimationClip(str); }, MonoString* a_path) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, CreateSkeletonBuffer, { return Instance->CreateSkeletonBuffer(); }) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroySkeletonBuffer, { Instance->DestroySkeletonBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, ClearSkeletonBuffer, { Instance->ClearSkeletonBuffer(a_addr); }, uint32_t a_addr) \

ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

RUNTIME_FUNCTION(RuntimeBoneData, Skeleton, LoadBoneData,
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    RuntimeBoneData data;

    data.BindPoses = NULL;
    data.Names = NULL;
    data.Parents = NULL;

    std::vector<BoneData> bones;
    if (FlareBase::ColladaLoader_LoadBoneFile(str, &bones))
    {
        MonoDomain* domain = RuntimeManager::GetDomain();
        MonoClass* fClass = mono_get_single_class();

        const uint32_t count = (uint32_t)bones.size();
        data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)count);
        data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)count);
        data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)count);

        for (uint32_t i = 0; i < count; ++i)
        {
            const BoneData& bone = bones[i];

            MonoArray* bindPose = mono_array_new(domain, fClass, 16);
            for (uint32_t j = 0; j < 16; ++j)
            {
                mono_array_set(bindPose, float, j, bone.Transform[j / 4][j % 4]);
            }

            mono_array_set(data.BindPoses, MonoArray*, i, bindPose);
            mono_array_set(data.Names, MonoString*, i, mono_string_new(domain, bone.Name.c_str()));
            mono_array_set(data.Parents, uint32_t, i, bone.Parent);
        }
    }

    return data;
}, MonoString* a_path)

RUNTIME_FUNCTION(void, SkinnedMeshRenderer, PushBoneData,
{
    glm::mat4 bindPose;

    float* f = (float*)&bindPose;

    for (uint32_t i = 0; i < 16; ++i)
    {
        f[i] = mono_array_get(a_bindPose, float, i);
    }

    Instance->PushSkeletonBoneData(a_addr, a_transformIndex, bindPose);
}, uint32_t a_addr, uint32_t a_transformIndex, MonoArray* a_bindPose)

AnimationControllerBindings::AnimationControllerBindings(AnimationController* a_controller)
{
    TRACE("Binding AnimationController functions to C#");
    Instance = this;

    m_controller = a_controller;

    BIND_FUNCTION(IcarianEngine.Rendering.Animation, Skeleton, LoadBoneData);

    BIND_FUNCTION(IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, PushBoneData);

    ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
}
AnimationControllerBindings::~AnimationControllerBindings()
{

}

uint32_t AnimationControllerBindings::GenerateAnimatorBuffer() const
{
    return m_controller->m_animators.PushVal(AnimationUpdateMode_Update);
}
void AnimationControllerBindings::DestroyAnimatorBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_controller->m_animators.Size(), "DestroyAnimatorBuffer out of bounds");

    m_controller->m_animators.Erase(a_addr);
}
e_AnimationUpdateMode AnimationControllerBindings::GetAnimatorUpdateMode(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_controller->m_animators.Size(), "GetAnimatorUpdateMode out of bounds");
    ICARIAN_ASSERT_MSG(m_controller->m_animators.Exists(a_addr), "GetAnimatorUpdateMode value does not exist");

    return m_controller->m_animators[a_addr];
}
void AnimationControllerBindings::SetAnimatorUpdateMode(uint32_t a_addr, e_AnimationUpdateMode a_updateMode) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_controller->m_animators.Size(), "SetAnimatorUpdateMode out of bounds");
    ICARIAN_ASSERT_MSG(m_controller->m_animators.Exists(a_addr), "SetAnimatorUpdateMode value does not exist");

    m_controller->m_animators.LockSet(a_addr, a_updateMode);
}

uint32_t AnimationControllerBindings::CreateSkeletonBuffer() const
{
    SkeletonData data;

    return m_controller->m_skeletons.PushVal(data);
}
void AnimationControllerBindings::DestroySkeletonBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_controller->m_skeletons.Size(), "DestroySkeletonBuffer out of bounds");

    m_controller->m_skeletons.Erase(a_addr);
}
void AnimationControllerBindings::ClearSkeletonBuffer(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_controller->m_skeletons.Size(), "ClearSkeletonBuffer out of bounds");

    TLockArray<SkeletonData> a = m_controller->m_skeletons.ToLockArray();
    a[a_addr].BoneData.clear();
}
void AnimationControllerBindings::PushSkeletonBoneData(uint32_t a_addr, uint32_t a_transformIndex, const glm::mat4& a_inverseBindPose) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_controller->m_skeletons.Size(), "PushSkeletonBoneData out of bounds");

    BoneTransformData data;
    data.TransformIndex = a_transformIndex;
    data.InverseBindPose = a_inverseBindPose;

    TLockArray<SkeletonData> a = m_controller->m_skeletons.ToLockArray();
    a[a_addr].BoneData.push_back(data);
}

MonoArray* AnimationControllerBindings::LoadAnimationClip(const std::filesystem::path& a_path) const
{
    MonoArray* data = NULL;

    MonoDomain* domain = RuntimeManager::GetDomain();
    MonoClass* animationDataClass = RuntimeManager::GetClass("IcarianEngine.Rendering.Animation", "AnimationData");
    ICARIAN_ASSERT(animationDataClass != NULL);
    MonoClass* animationFrameClass = RuntimeManager::GetClass("IcarianEngine.Rendering.Animation", "AnimationFrame");
    ICARIAN_ASSERT(animationFrameClass != NULL);
    MonoClass* floatClass = mono_get_single_class();

    std::vector<ColladaAnimationData> animations;
    if (FlareBase::ColladaLoader_LoadAnimationFile(a_path, &animations))
    {
        const uint32_t count = (uint32_t)animations.size();
        data = mono_array_new(domain, animationDataClass, (uintptr_t)count);
        for (uint32_t i = 0; i < count; ++i)
        {
            const ColladaAnimationData& animation = animations[i];

            RuntimeAnimationData animData;
            animData.Name = mono_string_new(domain, animation.Name.c_str());

            const uint32_t frameCount = (uint32_t)animation.Frames.size();
            animData.Frames = mono_array_new(domain, animationFrameClass, (uintptr_t)frameCount);
            for (uint32_t j = 0; j < frameCount; ++j)
            {
                const ColladaAnimationFrame& frame = animation.Frames[j];

                RuntimeAnimationFrame animFrame;
                animFrame.Time = frame.Time;

                const float* t = (float*)&frame.Transform;

                MonoArray* transform = mono_array_new(domain, floatClass, 16);
                for (uint32_t k = 0; k < 16; ++k)
                {
                    mono_array_set(transform, float, k, t[k]);
                }

                animFrame.Transform = transform;

                mono_array_set(animData.Frames, RuntimeAnimationFrame, j, animFrame);
            }

            mono_array_set(data, RuntimeAnimationData, i, animData);
        }
    }

    return data;
}