#ifndef INCLUDED_HEADER_BUILDICARIANCS
#define INCLUDED_HEADER_BUILDICARIANCS

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

static CUBE_CSProject BuildIcarianCSProject(CBBOOL a_optimise)
{
    CUBE_CSProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianCS");
    project.Target = CUBE_CSProjectTarget_Exe;
    project.OutputPath = CUBE_Path_CreateC("./build/");
    project.Optimise = a_optimise;

    CUBE_CSProject_AppendIncludePath(&project, "../EngineInterop");

    CUBE_CSProject_AppendSources(&project, 
        "src/Application.cs",
        "src/AssetLibrary.cs",
        "src/Component.cs",
        "src/EditorPathStringAttribute.cs",
        "src/EditorTooltipAttribute.cs",
        "src/Extensions.cs",
        "src/GameObject.cs",
        "src/HideInEditorAttribute.cs",
        "src/IDestroy.cs",
        "src/Input.cs",
        "src/Logger.cs",
        "src/NativeLock.cs",
        "src/PrimitiveGenerator.cs",
        "src/Profiler.cs",
        "src/Program.cs",
        "src/Random.cs",
        "src/Scene.cs",
        "src/Scribe.cs",
        "src/Scriptable.cs",
        "src/ThreadPool.cs",
        "src/Time.cs",
        "src/Transform.cs",

        "src/Audio/AudioClip.cs",
        "src/Audio/AudioListener.cs",
        "src/Audio/AudioMixer.cs",
        "src/Audio/AudioSource.cs",

        "src/Definitions/AmbientLightDef.cs",
        "src/Definitions/AnimationControllerDef.cs",
        "src/Definitions/AnimatorDef.cs",
        "src/Definitions/AudioSourceDef.cs",
        "src/Definitions/BoxCollisionShapeDef.cs",
        "src/Definitions/CameraDef.cs",
        "src/Definitions/CapsuleCollisionShapeDef.cs",
        "src/Definitions/CollisionShapeDef.cs",
        "src/Definitions/ComponentDef.cs",
        "src/Definitions/CylinderCollisionShapeDef.cs",
        "src/Definitions/Def.cs",
        "src/Definitions/DefLibrary.cs",
        "src/Definitions/DefTableAttribute.cs",
        "src/Definitions/DirectionalLightDef.cs",
        "src/Definitions/GameObjectDef.cs",
        "src/Definitions/LightDef.cs",
        "src/Definitions/MaterialDef.cs",
        "src/Definitions/MeshRendererDef.cs",
        "src/Definitions/ParticleSystem2DDef.cs",
        "src/Definitions/ParticleSystemDef.cs",
        "src/Definitions/PhysicsBodyDef.cs",
        "src/Definitions/PointLightDef.cs",
        "src/Definitions/RendererDef.cs",
        "src/Definitions/RigidBodyDef.cs",
        "src/Definitions/SkeletonAnimatorDef.cs",
        "src/Definitions/SkeletonClipAnimationControllerDef.cs",
        "src/Definitions/SkinnedMeshRendererDef.cs",
        "src/Definitions/SphereCollisionShapeDef.cs",
        "src/Definitions/SpotLightDef.cs",
        "src/Definitions/TriggerBodyDef.cs",

        "src/Maths/Color.cs",
        "src/Maths/IVector2.cs",
        "src/Maths/IVector3.cs",
        "src/Maths/IVector4.cs",
        "src/Maths/Mathf.cs",
        "src/Maths/Matrix4.cs",
        "src/Maths/Quaternion.cs",
        "src/Maths/Vector2.cs",
        "src/Maths/Vector3.cs",
        "src/Maths/Vector4.cs",

        "src/Mod/AssemblyControl.cs",
        "src/Mod/IcarianAssembly.cs",
        "src/Mod/IcarianAssemblyInfo.cs",
        "src/Mod/ModControl.cs",

        "src/Networking/NetworkClient.cs",
        "src/Networking/NetworkManager.cs",
        "src/Networking/NetworkServer.cs",
        "src/Networking/NetworkSocket.cs",

        "src/Physics/Physics.cs",
        "src/Physics/PhysicsBody.cs",
        "src/Physics/RigidBody.cs",
        "src/Physics/TriggerBody.cs",
        
        "src/Physics/Shapes/BoxCollisionShape.cs",
        "src/Physics/Shapes/CapsuleCollisionShape.cs",
        "src/Physics/Shapes/CollisionShape.cs",
        "src/Physics/Shapes/CylinderCollisionShape.cs",
        "src/Physics/Shapes/SphereCollisionShape.cs",

        "src/Rendering/Camera.cs",
        "src/Rendering/DefaultRenderPipeline.cs",
        "src/Rendering/DepthCubeRenderTexture.cs",
        "src/Rendering/DepthRenderTexture.cs",
        "src/Rendering/IRenderTexture.cs",
        "src/Rendering/Material.cs",
        "src/Rendering/MeshRenderer.cs",
        "src/Rendering/Model.cs",
        "src/Rendering/MultiRenderTexture.cs",
        "src/Rendering/ParticleSystem.cs",
        "src/Rendering/ParticleSystem2D.cs",
        "src/Rendering/PixelShader.cs",
        "src/Rendering/RenderCommand.cs",
        "src/Rendering/Renderer.cs",
        "src/Rendering/RenderPipeline.cs",
        "src/Rendering/RenderTexture.cs",
        "src/Rendering/RenderTextureCmd.cs",
        "src/Rendering/Texture.cs",
        "src/Rendering/TextureSampler.cs",
        "src/Rendering/VertexShader.cs",
        "src/Rendering/Viewport.cs",

        "src/Rendering/Animation/AnimationClip.cs",
        "src/Rendering/Animation/AnimationController.cs",
        "src/Rendering/Animation/Animator.cs",
        "src/Rendering/Animation/Skeleton.cs",
        "src/Rendering/Animation/SkeletonAnimator.cs",
        "src/Rendering/Animation/SkeletonClipAnimationController.cs",
        "src/Rendering/Animation/SkinnedMeshRenderer.cs",

        "src/Rendering/Lighting/AmbientLight.cs",
        "src/Rendering/Lighting/DirectionalLight.cs",
        "src/Rendering/Lighting/Light.cs",
        "src/Rendering/Lighting/PointLight.cs",
        "src/Rendering/Lighting/SpotLight.cs",

        "src/Rendering/PostEffects/EmissionPostEffect.cs",
        "src/Rendering/PostEffects/PostEffect.cs",
        "src/Rendering/PostEffects/ToneMapPostEffect.cs",

        "src/Rendering/UI/Canvas.cs",
        "src/Rendering/UI/CanvasRenderer.cs",
        "src/Rendering/UI/Font.cs",
        "src/Rendering/UI/ImageUIElement.cs",
        "src/Rendering/UI/TextUIElement.cs",
        "src/Rendering/UI/UIElement.cs"
    );

    return project;
}

#ifdef __cplusplus
}
#endif

#endif