#ifndef INCLUDED_HEADER_BUILDICARIANCS
#define INCLUDED_HEADER_BUILDICARIANCS

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CSProject BuildIcarianCSProject(CBBOOL a_optimise)
{
    CUBE_CSProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianCS");
    project.Target = CUBE_CSProjectTarget_Exe;
    project.OutputPath = CUBE_Path_CreateC("./build/");
    project.Optimise = a_optimise;

    CUBE_CSProject_AppendSource(&project, "src/Application.cs");
    CUBE_CSProject_AppendSource(&project, "src/AssetLibrary.cs");
    CUBE_CSProject_AppendSource(&project, "src/Component.cs");
    CUBE_CSProject_AppendSource(&project, "src/EditorTooltipAttribute.cs");
    CUBE_CSProject_AppendSource(&project, "src/GameObject.cs");
    CUBE_CSProject_AppendSource(&project, "src/HideInEditorAttribute.cs");
    CUBE_CSProject_AppendSource(&project, "src/IDestroy.cs");
    CUBE_CSProject_AppendSource(&project, "src/Input.cs");
    CUBE_CSProject_AppendSource(&project, "src/Logger.cs");
    CUBE_CSProject_AppendSource(&project, "src/NativeLock.cs");
    CUBE_CSProject_AppendSource(&project, "src/PrimitiveGenerator.cs");
    CUBE_CSProject_AppendSource(&project, "src/Profiler.cs");
    CUBE_CSProject_AppendSource(&project, "src/Program.cs");
    CUBE_CSProject_AppendSource(&project, "src/Scene.cs");
    CUBE_CSProject_AppendSource(&project, "src/Scribe.cs");
    CUBE_CSProject_AppendSource(&project, "src/Scriptable.cs");
    CUBE_CSProject_AppendSource(&project, "src/ThreadPool.cs");
    CUBE_CSProject_AppendSource(&project, "src/Time.cs");
    CUBE_CSProject_AppendSource(&project, "src/Transform.cs");

    CUBE_CSProject_AppendSource(&project, "src/Audio/AudioClip.cs");
    CUBE_CSProject_AppendSource(&project, "src/Audio/AudioListener.cs");
    CUBE_CSProject_AppendSource(&project, "src/Audio/AudioMixer.cs");
    CUBE_CSProject_AppendSource(&project, "src/Audio/AudioSource.cs");

    CUBE_CSProject_AppendSource(&project, "src/Definitions/AnimationControllerDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/AnimatorDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/AudioSourceDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/BoxCollisionShapeDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/CameraDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/CapsuleCollisionShapeDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/CollisionShapeDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/ComponentDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/CylinderCollisionShapeDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/Def.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/DefLibrary.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/DefTableAttribute.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/DirectionalLightDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/GameObjectDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/LightDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/MaterialDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/MeshRendererDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/PhysicsBodyDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/PointLightDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/RendererDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/RigidBodyDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/SkeletonAnimatorDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/SkeletonClipAnimationControllerDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/SkinnedMeshRendererDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/SphereCollisionShapeDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/SpotLightDef.cs");
    CUBE_CSProject_AppendSource(&project, "src/Definitions/TriggerBodyDef.cs");

    CUBE_CSProject_AppendSource(&project, "src/Maths/Color.cs");
    CUBE_CSProject_AppendSource(&project, "src/Maths/Mathf.cs");
    CUBE_CSProject_AppendSource(&project, "src/Maths/Matrix4.cs");
    CUBE_CSProject_AppendSource(&project, "src/Maths/Quaternion.cs");
    CUBE_CSProject_AppendSource(&project, "src/Maths/Vector2.cs");
    CUBE_CSProject_AppendSource(&project, "src/Maths/Vector3.cs");
    CUBE_CSProject_AppendSource(&project, "src/Maths/Vector4.cs");

    CUBE_CSProject_AppendSource(&project, "src/Mod/AssemblyControl.cs");
    CUBE_CSProject_AppendSource(&project, "src/Mod/IcarianAssembly.cs");
    CUBE_CSProject_AppendSource(&project, "src/Mod/IcarianAssemblyInfo.cs");
    CUBE_CSProject_AppendSource(&project, "src/Mod/ModControl.cs");

    CUBE_CSProject_AppendSource(&project, "src/Physics/Physics.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/PhysicsBody.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/RigidBody.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/TriggerBody.cs");

    CUBE_CSProject_AppendSource(&project, "src/Physics/Shapes/BoxCollisionShape.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/Shapes/CapsuleCollisionShape.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/Shapes/CollisionShape.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/Shapes/CylinderCollisionShape.cs");
    CUBE_CSProject_AppendSource(&project, "src/Physics/Shapes/SphereCollisionShape.cs");

    CUBE_CSProject_AppendSource(&project, "src/Rendering/Camera.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/DefaultRenderPipeline.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/DepthRenderTexture.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/IRenderTexture.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Material.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/MeshRenderer.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Model.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/MultiRenderTexture.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/PixelShader.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/RenderCommand.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Renderer.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/RenderPipeline.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/RenderTexture.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/RenderTextureCmd.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Texture.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/TextureSampler.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/VertexShader.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Viewport.cs");

    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/AnimationClip.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/AnimationController.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/Animator.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/Skeleton.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/SkeletonAnimator.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/SkeletonClipAnimationController.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Animation/SkinnedMeshRenderer.cs");

    CUBE_CSProject_AppendSource(&project, "src/Rendering/Lighting/DirectionalLight.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Lighting/Light.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Lighting/PointLight.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/Lighting/SpotLight.cs");

    CUBE_CSProject_AppendSource(&project, "src/Rendering/UI/Canvas.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/UI/CanvasRenderer.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/UI/Font.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/UI/ImageUIElement.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/UI/TextUIElement.cs");
    CUBE_CSProject_AppendSource(&project, "src/Rendering/UI/UIElement.cs");

    return project;
}

#ifdef __cplusplus
}
#endif

#endif