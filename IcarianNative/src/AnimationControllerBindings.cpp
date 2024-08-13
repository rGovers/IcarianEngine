// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/AnimationControllerBindings.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Rendering/AnimationController.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#include "EngineSkeletonInteropStructures.h"
#include "EngineAnimationClipInteropStructures.h"

static AnimationControllerBindings* Instance = nullptr;

#define ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GenerateBuffer, { return Instance->GenerateAnimatorBuffer(); }) \
    F(void, IcarianEngine.Rendering.Animation, Animator, DestroyBuffer, { Instance->DestroyAnimatorBuffer(a_addr); }, uint32_t a_addr) \
    F(uint32_t, IcarianEngine.Rendering.Animation, Animator, GetUpdateMode, { return (uint32_t)Instance->GetAnimatorUpdateMode(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, Animator, SetUpdateMode, { Instance->SetAnimatorUpdateMode(a_addr, (e_AnimationUpdateMode)a_updateMode); }, uint32_t a_addr, uint32_t a_updateMode) \
    \
    F(uint32_t, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, CreateSkeletonBuffer, { return Instance->CreateSkeletonBuffer(); }) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, DestroySkeletonBuffer, { Instance->DestroySkeletonBuffer(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, ClearSkeletonBuffer, { Instance->ClearSkeletonBuffer(a_addr); }, uint32_t a_addr) \

ANIMATIONCONTROLLER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(RuntimeImportBoneData, Skeleton, LoadBoneData,
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path path = std::filesystem::path(str);
    const std::filesystem::path ext = path.extension();

    RuntimeImportBoneData data = { 0 };

    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str()))
    {
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(path);
        IVERIFY(handle != nullptr);
        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        uint8_t* dat = new uint8_t[size];
        IDEFER(delete[] dat);

        if (handle->Read(dat, size) != size)
        {
            IERROR("Failed to read Skeleton file: " + path.string());

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, 0, extStr.c_str() + 1);
        IVERIFY(scene != nullptr);

        if (scene->mNumSkeletons <= 0)
        {
            IWARN("No skeleton");

            break;
        }

        const aiSkeleton* skeleton = scene->mSkeletons[0];

        const uint32_t boneCount = (uint32_t)skeleton->mNumBones;
        if (boneCount <= 0)
        {
            IWARN("No bones");

            break;
        }

        MonoDomain* domain = mono_domain_get();
        MonoClass* fClass = mono_get_single_class();

        data.BindPoses = mono_array_new(domain, mono_get_array_class(), (uintptr_t)boneCount);
        data.Names = mono_array_new(domain, mono_get_string_class(), (uintptr_t)boneCount);
        data.Parents = mono_array_new(domain, mono_get_uint32_class(), (uintptr_t)boneCount);

        for (uint32_t i = 0; i < boneCount; ++i)
        {
            const aiSkeletonBone* bone = skeleton->mBones[i];

            aiMatrix4x4 bindPose = bone->mOffsetMatrix;
            bindPose.Inverse();

            MonoArray* bindPoseArr = mono_array_new(domain, fClass, 16);
            for (uint32_t j = 0; j < 16; ++j)
            {
                mono_array_set(bindPoseArr, float, j, bindPose[j / 4][j % 4]);
            }

            mono_array_set(data.BindPoses, MonoArray*, i, bindPoseArr);
            mono_array_set(data.Names, MonoString*, i, mono_string_new(domain, bone->mNode->mName.C_Str()));
            mono_array_set(data.Parents, uint32_t, i, (uint32_t)bone->mParent);
        }

        break;
    }
    default:
    {
        IERROR("Invalid Skeleton file extension: " + path.string());

        break;
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

RUNTIME_FUNCTION(MonoArray*, AnimationClip, LoadExternalAnimationData, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const std::filesystem::path path = std::filesystem::path(str);
    const std::filesystem::path ext = path.extension();
    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str()))
    {
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(path);
        IVERIFY(handle != nullptr);
        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        uint8_t* dat = new uint8_t[size];
        IDEFER(delete[] dat);

        if (handle->Read(dat, size) != size)
        {
            IERROR("Failed to read external animation clip file: " + path.string());

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, 0, extStr.c_str() + 1);
        IVERIFY(scene != nullptr);

        if (scene->mNumAnimations <= 0)
        {
            break;
        }

        const aiAnimation* animation = scene->mAnimations[0];

        Array<AnimationDataExternal> dataArray;

        MonoDomain* domain = RuntimeManager::GetDomain();
        MonoClass* frameClass = RuntimeManager::GetClass("IcarianEngine.Rendering.Animation", "AnimaionFrameExternal");
        IVERIFY(frameClass != NULL);

        const uint32_t channelCount = (uint32_t)animation->mNumChannels;
        for (uint32_t i = 0; i < channelCount; ++i)
        {
            const aiNodeAnim* anim = animation->mChannels[i];

            const uint32_t posCount = (uint32_t)anim->mNumPositionKeys;
            if (posCount > 0)
            {
                AnimationDataExternal dat;
                dat.Name = mono_string_new(domain, anim->mNodeName.C_Str());
                dat.Target = mono_string_new(domain, "Translation");
                dat.Frames = mono_array_new(domain, frameClass, (uintptr_t)posCount);

                for (uint32_t j = 0; j < posCount; ++j)
                {
                    const aiVectorKey& posKey = anim->mPositionKeys[j];

                    AnimationFrameExternal frame;
                    frame.Time = (float)posKey.mTime;
                    frame.Data = glm::vec4(posKey.mValue.x, -posKey.mValue.y, posKey.mValue.z, 1.0f);

                    mono_array_set(dat.Frames, AnimationFrameExternal, j, frame);
                }

                dataArray.Push(dat);
            }

            const uint32_t rotationCount = (uint32_t)anim->mNumRotationKeys;
            if (rotationCount > 0)
            {
                AnimationDataExternal dat;
                dat.Name = mono_string_new(domain, anim->mNodeName.C_Str());
                dat.Target = mono_string_new(domain, "Rotation");
                dat.Frames = mono_array_new(domain, frameClass, (uintptr_t)rotationCount);

                for (uint32_t j = 0; j < rotationCount; ++j)
                {
                    const aiQuatKey& rotKey = anim->mRotationKeys[j];

                    AnimationFrameExternal frame;
                    frame.Time = (float)rotKey.mTime;
                    frame.Data = glm::vec4(rotKey.mValue.x, -rotKey.mValue.y, rotKey.mValue.z, rotKey.mValue.z);

                    mono_array_set(dat.Frames, AnimationFrameExternal, j, frame);
                }

                dataArray.Push(dat);
            }

            const uint32_t scaleCount = (uint32_t)anim->mNumScalingKeys;
            if (scaleCount > 0)
            {
                AnimationDataExternal dat;
                dat.Name = mono_string_new(domain, anim->mNodeName.C_Str());
                dat.Target = mono_string_new(domain, "Scale");
                dat.Frames = mono_array_new(domain, frameClass, (uintptr_t)scaleCount);

                for (uint32_t j = 0; j < scaleCount; ++j)
                {
                    const aiVectorKey& scaleKey = anim->mScalingKeys[j];

                    AnimationFrameExternal frame;
                    frame.Time = (float)scaleKey.mTime;
                    frame.Data = glm::vec4(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z, 0.0f);

                    mono_array_set(dat.Frames, AnimationFrameExternal, j, frame);
                }

                dataArray.Push(dat);
            }
        }

        const uint32_t dataCount = dataArray.Size();
        if (dataCount > 0)
        {
            MonoClass* dataClass = RuntimeManager::GetClass("IcarianEngine.Rendering.Animation", "AnimationDataExternal");
            IVERIFY(dataClass != NULL);

            MonoArray* array = mono_array_new(domain, dataClass, (uintptr_t)dataCount);

            for (uint32_t i = 0; i < dataCount; ++i)
            {
                mono_array_set(array, AnimationDataExternal, i, dataArray[i]);
            }

            return array;
        }

        break;
    }
    default:
    {
        IERROR("Invalid external animation clip extension: " + path.string());

        break;
    }
    }

    return NULL;
}, MonoString* a_path)

AnimationControllerBindings::AnimationControllerBindings(AnimationController* a_controller)
{
    TRACE("Binding AnimationController functions to C#");
    Instance = this;

    m_controller = a_controller;

    BIND_FUNCTION(IcarianEngine.Rendering.Animation, Skeleton, LoadBoneData);

    BIND_FUNCTION(IcarianEngine.Rendering.Animation, SkinnedMeshRenderer, PushBoneData);

    BIND_FUNCTION(IcarianEngine.Rendering.Animation, AnimationClip, LoadExternalAnimationData);

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