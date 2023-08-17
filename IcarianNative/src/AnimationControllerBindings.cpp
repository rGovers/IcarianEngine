#include "Rendering/AnimtionControllerBindings.h"

#include "Flare/ColladaLoader.h"
#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/AnimationController.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static AnimationControllerBindings* Instance = nullptr;

#define ANIMATIONCONTROLLER_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(a_runtime, namespace, klass, name);

struct RuntimeBoneData
{
    MonoArray* Names;
    MonoArray* Parents;
    MonoArray* BindPoses;
};

#define ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GenerateBuffer, { return Instance->GenerateAnimatorBuffer(); }) \
    F(void, IcarianEngine.Rendering.Animation, Animator, DestroyBuffer, { Instance->DestroyAnimatorBuffer(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GetUpdateMode, { return (uint32_t)Instance->GetAnimatorUpdateMode(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, Animator, SetUpdateMode, { Instance->SetAnimatorUpdateMode(a_addr, (e_AnimationUpdateMode)a_updateMode); }, uint32_t a_addr, uint32_t a_updateMode) 

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
        MonoDomain* domain = mono_domain_get();
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

AnimationControllerBindings::AnimationControllerBindings(AnimationController* a_controller, RuntimeManager* a_runtime)
{
    TRACE("Binding AnimationController functions to C#");
    m_controller = a_controller;

    Instance = this;

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.Animation, Skeleton, LoadBoneData);

    ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(ANIMATIONCONTROLLER_RUNTIME_ATTACH);
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
