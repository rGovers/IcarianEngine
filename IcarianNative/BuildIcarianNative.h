#ifndef INCLUDED_HEADER_BUILDICARIANNATIVE
#define INCLUDED_HEADER_BUILDICARIANNATIVE

#ifdef __cplusplus
extern "C" {
#endif

#include "CUBE/CUBE.h"

#include "../BuildBase.h"
#include "lib/BuildIcarianNativeDependencies.h"

const static char* IcarianNativeShaderBasePaths[] =
{
    "./shaders/AmbientOcclusion.fpix",
    "./shaders/AmbientOcclusionFilter.fpix",
    "./shaders/AmbientLight.fpix",
    "./shaders/Blend.fpix",
    "./shaders/DirectionalLight.fpix",
    "./shaders/PointLight.fpix",
    "./shaders/PostAtmosphere.fpix",
    "./shaders/PostEmission.fpix",
    "./shaders/PostEmissionBlur.fpix",
    "./shaders/PostToneMap.fpix",
    "./shaders/Quad.vert",
    "./shaders/ShadowDirectionalLight.fpix",
    "./shaders/ShadowPointLight.fpix",
    "./shaders/ShadowSpotLight.fpix",
    "./shaders/SpotLight.fpix",
    "./shaders/UI.fvert",
    "./shaders/UIImage.fpix",
    "./shaders/UIText.fpix",
    "./shaders/Particle.ftask"
};

const static CBUINT32 IcarianNativeShaderBasePathCount = sizeof(IcarianNativeShaderBasePaths) / sizeof(*IcarianNativeShaderBasePaths);

static CBBOOL WriteIcarianNativeShadersToHeader(const char* a_workingPath)
{
    CUBE_Path workingPath = CUBE_Path_CreateC(a_workingPath);

    CUBE_Path shaderPaths[IcarianNativeShaderBasePathCount];

    for (CBUINT32 i = 0; i < IcarianNativeShaderBasePathCount; ++i)
    {
        shaderPaths[i] = CUBE_Path_CombineC(&workingPath, IcarianNativeShaderBasePaths[i]);
    }

    CUBE_Path outPath = CUBE_Path_CombineC(&workingPath, "include/Shaders.h");
    CUBE_String outPathStr = CUBE_Path_ToString(&outPath);

    const CBBOOL ret = ShadersToHeader(shaderPaths, IcarianNativeShaderBasePathCount, outPathStr.Data);

    CUBE_String_Destroy(&outPathStr);
    CUBE_Path_Destroy(&outPath);

    for (CBUINT32 i = 0; i < IcarianNativeShaderBasePathCount; ++i)
    {
        CUBE_Path_Destroy(&shaderPaths[i]);
    }

    CUBE_Path_Destroy(&workingPath);

    return ret;
}

typedef struct
{
    CBBOOL EnableTrace;
    CBBOOL EnableProfiler;
    CBBOOL EnableMarkers;
    CBBOOL RemoteMode;
    CBBOOL EnablePipeFile;
} IcarianNativeProjectFlags;

static CUBE_CProject BuildIcarianNativeProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration, IcarianNativeProjectFlags a_flags)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianNative");
    project.Target = CUBE_CProjectTarget_Exe;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_String commitDefine = CUBE_String_CreateC("ICARIANNATIVE_COMMIT_HASH="); 
    if (a_targetPlatform != TargetPlatform_LinuxSteam)
    {
        CUBE_StackString commitHash = CUBE_Git_GetCommitHashShort();

        CUBE_String_AppendSS(&commitDefine, &commitHash);
    }
    else
    {
        CUBE_String_AppendC(&commitDefine, "Steam");
    }

    CUBE_CProject_AppendDefines(&project,
        "ICARIANNATIVE_VERSION_MAJOR=2025",
        "ICARIANNATIVE_VERSION_MINOR=0",
        "ICARIANNATIVE_VERSION_PATCH=0",
        commitDefine.Data,
        "ICARIANNATIVE_VERSION_TAG=DEV",

        "VK_NO_PROTOTYPES",
        "GLM_FORCE_QUAT_DATA_XYZW",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_FORCE_RADIANS",
        "ENABLE_OPT=1",
        "AL_LIBTYPE_STATIC",
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2"
    );

    CUBE_String_Destroy(&commitDefine);

    if (a_flags.EnableMarkers)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_MARKERS");
    }
    if (a_flags.EnableTrace)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_TRACE");
    }
    if (a_flags.EnableProfiler)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_PROFILER");
    }
    if (a_flags.EnablePipeFile)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_PIPEFILE");
    }

    // This is still an experimental feature but enabling it to start testing
    // We do want this feature is it allows a 4x to 10x speed up with the swapchain in headless mode when running on the same system
    // (We mostly use headless mode when using the editors window and not our own and the performance hit is due to process boundaries and data transfer)
    // Windows is still giving me issues and not setup to test properly on Windows
    // And disabling in remote mode because well it is a remote system so DMA is imposible
    if (!a_flags.RemoteMode && a_targetPlatform != TargetPlatform_Windows)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_DMA");
    }

    CUBE_CProject_AppendIncludePaths(&project, 
        "./include",

        "../EngineInterop",

        "../IcarianCore/include",

        "../deps/assimp/include",
        "../deps/gen/assimp",
        "../deps/glfw/include",
        "../deps/flare-glm",
        "../deps/stb",
        "../deps/KTX-Software/include",
        "../deps/flare-tinyxml2",
	    "../deps/Vulkan-Headers/include",
        "../deps/renderdoc/app/",
        "../deps/enet/include",
        "../deps/meshoptimizer/src",

        "./lib/glslang",
        "./lib/glslang/External/spirv-tools/include",
        "./lib/JoltPhysics",
        "./lib/minimp4",
        "./lib/SPIRV-Tools/include",
        "./lib/VulkanMemoryAllocator/include",
        "./lib/miniaudio"
    );

    CUBE_CProject_AppendSources(&project, 
        "../deps/flare-tinyxml2/tinyxml2.cpp",

        "./src/main.cpp",

        "./src/AnimationController.cpp",
        "./src/AnimationControllerBindings.cpp",
        "./src/Application.cpp",
        "./src/AudioEngine.cpp",
        "./src/AudioEngineBindings.cpp",
        "./src/CacheFileHandle.cpp",
        "./src/Config.cpp",
        "./src/DeletionQueue.cpp",
        "./src/FileCache.cpp",
        "./src/Font.cpp",
        "./src/GamePad.cpp",
        "./src/GLFWAppWindow.cpp",
        "./src/H264.cpp",
        "./src/H264VideoInfo.cpp",
        "./src/HeadlessAppWindow.cpp",
        "./src/IcarianError.cpp",
        "./src/IcBodyActivationListener.cpp",
        "./src/IcBroadPhaseLayerInterface.cpp",
        "./src/IcCharacterListener.cpp",
        "./src/IcContactListener.cpp",
        "./src/IcObjectLayerPairFilter.cpp",
        "./src/IcObjectVsBroadPhaseLayerFilter.cpp",
        "./src/IcPhysicsJobSystem.cpp",
        "./src/ImageUIElement.cpp",
        "./src/InputManager.cpp",
        "./src/Logger.cpp",
        "./src/MaterialRenderStack.cpp",
        "./src/Navigation.cpp",
        "./src/NavigationBindings.cpp",
        "./src/NavigationMesh.cpp",
        "./src/NetworkClient.cpp",
        "./src/NetworkManager.cpp",
        "./src/NetworkServer.cpp",
        "./src/NullRenderEngineBackend.cpp",
        "./src/ObjectManager.cpp",
        "./src/OGGAudioClip.cpp",
        "./src/PhysicsEngine.cpp",
        "./src/PhysicsEngineBindings.cpp",
        "./src/PipeFileHandle.cpp",
        "./src/Profiler.cpp",
        "./src/Random.cpp",
        "./src/ReadFileHandle.cpp",
        "./src/RenderAssetStore.cpp",
        "./src/RenderAssetStoreBindings.cpp",
        "./src/RenderEngine.cpp",
        "./src/RuntimeFunction.cpp",
        "./src/RuntimeManager.cpp",
        "./src/RuntimeThreadJob.cpp",
        "./src/ShaderTable.cpp",
        "./src/SPIRVTools.cpp",
        "./src/TextUIElement.cpp",
        "./src/ThreadPool.cpp",
        "./src/UIControl.cpp",
        "./src/UIControlBindings.cpp",
        "./src/UIElement.cpp",
        "./src/VideoClip.cpp",
        "./src/VideoManager.cpp",
        "./src/VideoManagerBindings.cpp",

        "./src/WAVAudioClip.cpp"
    );

    CUBE_CProject_AppendRebuildSources(&project,
        "../IcarianCore/include/Core/Bitfield.h",
        "../IcarianCore/include/Core/CommunicationPipe.h",
        "../IcarianCore/include/Core/CRC.h",
        "../IcarianCore/include/Core/DMASwapBuffer.h",
        "../IcarianCore/include/Core/Endian.h",
        "../IcarianCore/include/Core/FlareShader.h",
        "../IcarianCore/include/Core/IcarianAssert.h",
        "../IcarianCore/include/Core/IcarianDefer.h",
        "../IcarianCore/include/Core/IcarianError.h",
        "../IcarianCore/include/Core/IcarianLambda.h",
        "../IcarianCore/include/Core/IcarianPragma.h",
        "../IcarianCore/include/Core/InputBindings.h",
        "../IcarianCore/include/Core/IPCPipe.h",
        "../IcarianCore/include/Core/LoggerHeader.h",
        "../IcarianCore/include/Core/MonoNativeImpl.h",
        "../IcarianCore/include/Core/Pipefile.h",
        "../IcarianCore/include/Core/PipeMessage.h",
        "../IcarianCore/include/Core/ShaderBuffers.h",
        "../IcarianCore/include/Core/SharedMemoryBuffer.h",
        "../IcarianCore/include/Core/SocketPipe.h",
        "../IcarianCore/include/Core/StringUtils.h",
        "../IcarianCore/include/Core/WindowsHeaders.h",

        "../EngineInterop/EngineAmbientLightInteropStructures.h",
        "../EngineInterop/EngineAnimationClipInteropStructures.h",
        "../EngineInterop/EngineApplicationInteropStructures.h",
        "../EngineInterop/EngineAudioClipInterop.h",
        "../EngineInterop/EngineAudioListenerInterop.h",
        "../EngineInterop/EngineAudioMixerInterop.h",
        "../EngineInterop/EngineAudioSourceInterop.h",
        "../EngineInterop/EngineAudioSourceInteropStructures.h",
        "../EngineInterop/EngineBoxCollisionShapeInterop.h",
        "../EngineInterop/EngineCanvasInterop.h",
        "../EngineInterop/EngineCanvasInteropStructures.h",
        "../EngineInterop/EngineCapsuleCollisionShapeInterop.h",
        "../EngineInterop/EngineCharacterControllerInterop.h",
        "../EngineInterop/EngineCollisionShapeInterop.h",
        "../EngineInterop/EngineCylinderCollisionShapeInterop.h",
        "../EngineInterop/EngineDirectionalLightInteropStructures.h",
        "../EngineInterop/EngineFontInterop.h",
        "../EngineInterop/EngineIcarianAssemblyInterop.h",
        "../EngineInterop/EngineImageUIElementInterop.h",
        "../EngineInterop/EngineInputInterop.h",
        "../EngineInterop/EngineInputInteropStructures.h",
        "../EngineInterop/EngineLightInteropStructures.h",
        "../EngineInterop/EngineMaterialInteropStructures.h",
        "../EngineInterop/EngineModelCollisionShapeInterop.h",
        "../EngineInterop/EngineNavigationMeshInterop.h",
        "../EngineInterop/EngineNetworkClientInterop.h",
        "../EngineInterop/EngineNetworkInteropStructures.h",
        "../EngineInterop/EngineNetworkServerInterop.h",
        "../EngineInterop/EngineParticleSystemInteropStructures.h",
        "../EngineInterop/EnginePhysicsBodyInterop.h",
        "../EngineInterop/EnginePhysicsBodyInteropStructures.h",
        "../EngineInterop/EnginePhysicsInterop.h",
        "../EngineInterop/EnginePhysicsInteropStructures.h",
        "../EngineInterop/EnginePointLightInteropStructures.h",
        "../EngineInterop/EngineRenderCommandInteropStructures.h",
        "../EngineInterop/EngineRigidBodyInterop.h",
        "../EngineInterop/EngineRigidBodyInteropStructures.h",
        "../EngineInterop/EngineSkeletonInteropStructures.h",
        "../EngineInterop/EngineSphereCollisionShapeInterop.h",
        "../EngineInterop/EngineSpotLightInteropStructures.h",
        "../EngineInterop/EngineTextUIElementInterop.h",
        "../EngineInterop/EngineTextureSamplerInteropStructures.h",
        "../EngineInterop/EngineTimeInterop.h",
        "../EngineInterop/EngineTransformInterop.h",
        "../EngineInterop/EngineTransformInteropStructures.h",
        "../EngineInterop/EngineTriggerBodyInterop.h",
        "../EngineInterop/EngineUIElementInterop.h",
        "../EngineInterop/EngineUIElementInteropStuctures.h",
        "../EngineInterop/EngineVideoClipInterop.h",
        "../EngineInterop/InteropBinding.h",
        "../EngineInterop/InteropTypes.h",

        "./include/AI/Navigation.h",
        "./include/AI/NavigationBindings.h",
        "./include/AI/NavigationMesh.h",

        "./include/AppWindow/AppWindow.h",
        "./include/AppWindow/GLFWAppWindow.h",
        "./include/AppWindow/HeadlessAppWindow.h",

        "./include/Audio/AudioClips/AudioClip.h",
        "./include/Audio/AudioClips/OGGAudioClip.h",
        "./include/Audio/AudioClips/WAVAudioClip.h",

        "./include/Audio/AudioEngine.h",
        "./include/Audio/AudioEngineBindings.h",
        "./include/Audio/AudioListenerBuffer.h",
        "./include/Audio/IcarianMiniaudio.h",

        "./include/DataTypes/Allocator.h",
        "./include/DataTypes/Array.h",
        "./include/DataTypes/BlockAllocator.h",
        "./include/DataTypes/RingAllocator.h",
        "./include/DataTypes/SpinLock.h",
        "./include/DataTypes/StackAllocator.h",
        "./include/DataTypes/TArray.h",
        "./include/DataTypes/ThreadGuard.h",
        "./include/DataTypes/TLockArray.h",
        "./include/DataTypes/TLockObj.h",
        "./include/DataTypes/TNCArray.h",
        "./include/DataTypes/TStatic.h",

        "./include/FileHandles/CacheFileHandle.h",
        "./include/FileHandles/FileHandle.h",
        "./include/FileHandles/PipeFileHandle.h",
        "./include/FileHandles/ReadFileHandle.h",

        "./include/Networking/NetworkClient.h",
        "./include/Networking/NetworkManager.h",
        "./include/Networking/NetworkServer.h",

        "./include/Physics/IcBodyActivationListener.h",
        "./include/Physics/IcBroadPhaseLayerInterface.h",
        "./include/Physics/IcCharacterListener.h",
        "./include/Physics/IcContactListener.h",
        "./include/Physics/IcObjectLayerPairFilter.h",
        "./include/Physics/IcObjectVsBroadPhaseLayerFilter.h",
        "./include/Physics/IcPhysicsJobSystem.h",
        "./include/Physics/InterfaceLock.h",
        "./include/Physics/PhysicsEngine.h",
        "./include/Physics/PhysicsEngineBindings.h",

        "./include/Rendering/Null/NullRenderEngineBackend.h",

        "./include/Rendering/UI/CanvasRendererBuffer.h",
        "./include/Rendering/UI/Font.h",
        "./include/Rendering/UI/ImageUIElement.h",
        "./include/Rendering/UI/TextUIElement.h",
        "./include/Rendering/UI/UIControl.h",
        "./include/Rendering/UI/UIControlBindings.h",
        "./include/Rendering/UI/UIElement.h",

        "./include/Rendering/Video/VideoInfo/H264VideoInfo.h",
        "./include/Rendering/Video/VideoInfo/VideoInfo.h",

        "./include/Rendering/Video/H264.h",
        "./include/Rendering/Video/VideoClip.h",
        "./include/Rendering/Video/VideoManager.h",
        "./include/Rendering/Video/VideoManagerBindings.h",

        "./include/Rendering/AnimationController.h",
        "./include/Rendering/AnimationControllerBindings.h",
        "./include/Rendering/CameraBuffer.h",
        "./include/Rendering/LibRenderDoc.h",
        "./include/Rendering/MaterialRenderStack.h",
        "./include/Rendering/RenderAssetStore.h",
        "./include/Rendering/RenderAssetStoreBindings.h",
        "./include/Rendering/RenderBuffers.h",
        "./include/Rendering/RenderDeviceInfo.h",
        "./include/Rendering/RenderEngine.h",
        "./include/Rendering/RenderEngineBackend.h",
        "./include/Rendering/ShaderTable.h",
        "./include/Rendering/SPIRVTools.h",
        "./include/Rendering/TextureData.h",
        "./include/Rendering/Viewport.h",

        "./include/Runtime/RuntimeFunction.h",
        "./include/Runtime/RuntimeManager.h",

        "./include/Application.h",
        "./include/Config.h",
        "./include/DeletionQueue.h",
        "./include/FileCache.h",
        "./include/Frustum.h",
        "./include/GamePad.h",
        "./include/IcarianError.h",
        "./include/InputManager.h",
        "./include/LibXInput.h",
        "./include/Logger.h",
        "./include/Memory.h",
        "./include/ObjectManager.h",
        "./include/Profiler.h",
        "./include/Random.h",
        "./include/RuntimeThreadJob.h",
        "./include/ThreadJob.h",
        "./include/ThreadPool.h",
        "./include/Trace.h",

        "./shaders/AmbientLight.fpix",
        "./shaders/AmbientOcclusion.fpix",
        "./shaders/AmbientOcclusionFilter.fpix",
        "./shaders/Blend.fpix",
        "./shaders/DirectionalLight.fpix",
        "./shaders/Particle.ftask",
        "./shaders/PointLight.fpix",
        "./shaders/PostAtmosphere.fpix",
        "./shaders/PostEmission.fpix",
        "./shaders/PostEmissionBlur.fpix",
        "./shaders/PostToneMap.fpix",
        "./shaders/Quad.vert",
        "./shaders/ShadowDirectionalLight.fpix",
        "./shaders/ShadowPointLight.fpix",
        "./shaders/ShadowSpotLight.fpix",
        "./shaders/SpotLight.fpix",
        "./shaders/UI.fvert",
        "./shaders/UIImage.fpix",
        "./shaders/UIText.fpix"
    );

    // Should probably make this separate but works for now
    if (!a_flags.RemoteMode)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC");
    }

    CUBE_CProject_AppendSource(&project, "./src/Library/LibRenderDoc.cpp");

    // Keeping it on for now just breaking it out in preperation for platform configuration
    if (1)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN");

        CUBE_CProject_AppendSources(&project,
            "./src/Platform/Vulkan/VulkanComputeEngine.cpp",
            "./src/Platform/Vulkan/VulkanComputeEngineBindings.cpp",
            "./src/Platform/Vulkan/VulkanComputeLayout.cpp",
            "./src/Platform/Vulkan/VulkanComputeParticle.cpp",
            "./src/Platform/Vulkan/VulkanComputePipeline.cpp",
            "./src/Platform/Vulkan/VulkanComputeShader.cpp",
            "./src/Platform/Vulkan/VulkanDepthCubeRenderTexture.cpp",
            "./src/Platform/Vulkan/VulkanDepthRenderTexture.cpp",
            "./src/Platform/Vulkan/VulkanGraphicsEngine.cpp",
            "./src/Platform/Vulkan/VulkanGraphicsEngineBindings.cpp",
            "./src/Platform/Vulkan/VulkanGraphicsParticle2D.cpp",
            "./src/Platform/Vulkan/VulkanLightData.cpp",
            "./src/Platform/Vulkan/VulkanMesh.cpp",
            "./src/Platform/Vulkan/VulkanMeshShader.cpp",
            "./src/Platform/Vulkan/VulkanModel.cpp",
            "./src/Platform/Vulkan/VulkanParticleShaderGenerator.cpp",
            "./src/Platform/Vulkan/VulkanPipeline.cpp",
            "./src/Platform/Vulkan/VulkanPixelShader.cpp",
            "./src/Platform/Vulkan/VulkanPushPool.cpp",
            "./src/Platform/Vulkan/VulkanRenderCommand.cpp",
            "./src/Platform/Vulkan/VulkanRenderEngineBackend.cpp",
            "./src/Platform/Vulkan/VulkanRenderTexture.cpp",
            "./src/Platform/Vulkan/VulkanShader.cpp",
            "./src/Platform/Vulkan/VulkanShaderData.cpp",
            "./src/Platform/Vulkan/VulkanShaderStorageObject.cpp",
            "./src/Platform/Vulkan/VulkanSwapchain.cpp",
            "./src/Platform/Vulkan/VulkanTaskShader.cpp",
            "./src/Platform/Vulkan/VulkanTexture.cpp",
            "./src/Platform/Vulkan/VulkanTextureSampler.cpp",
            "./src/Platform/Vulkan/VulkanUniformBuffer.cpp",
            "./src/Platform/Vulkan/VulkanVertexShader.cpp",
            "./src/Platform/Vulkan/VulkanVideoTexture.cpp",

            "./src/Library/LibVulkan.cpp"
        );

        CUBE_CProject_AppendRebuildSources(&project,
            "./include/Rendering/Vulkan/Shaders/VulkanComputeShader.h",
            "./include/Rendering/Vulkan/Shaders/VulkanMeshShader.h",
            "./include/Rendering/Vulkan/Shaders/VulkanPixelShader.h",
            "./include/Rendering/Vulkan/Shaders/VulkanShader.h",
            "./include/Rendering/Vulkan/Shaders/VulkanTaskShader.h",
            "./include/Rendering/Vulkan/Shaders/VulkanVertexShader.h",

            "./include/Rendering/Vulkan/IcarianVulkanHeader.h",
            "./include/Rendering/Vulkan/LibVulkan.h",
            "./include/Rendering/Vulkan/VulkanCommandBuffer.h",
            "./include/Rendering/Vulkan/VulkanComputeEngine.h",
            "./include/Rendering/Vulkan/VulkanComputeEngineBindings.h",
            "./include/Rendering/Vulkan/VulkanComputeLayout.h",
            "./include/Rendering/Vulkan/VulkanComputeParticle.h",
            "./include/Rendering/Vulkan/VulkanComputePipeline.h",
            "./include/Rendering/Vulkan/VulkanDepthCubeRenderTexture.h",
            "./include/Rendering/Vulkan/VulkanDepthRenderTexture.h",
            "./include/Rendering/Vulkan/VulkanGraphicsEngine.h",
            "./include/Rendering/Vulkan/VulkanGraphicsEngineBindings.h",
            "./include/Rendering/Vulkan/VulkanGraphicsParticle2D.h",
            "./include/Rendering/Vulkan/VulkanLightBuffer.h",
            "./include/Rendering/Vulkan/VulkanLightData.h",
            "./include/Rendering/Vulkan/VulkanMesh.h",
            "./include/Rendering/Vulkan/VulkanModel.h",
            "./include/Rendering/Vulkan/VulkanParticleShaderGenerator.h",
            "./include/Rendering/Vulkan/VulkanPipeline.h",
            "./include/Rendering/Vulkan/VulkanPushPool.h",
            "./include/Rendering/Vulkan/VulkanRenderCommand.h",
            "./include/Rendering/Vulkan/VulkanRenderEngineBackend.h",
            "./include/Rendering/Vulkan/VulkanRenderTexture.h",
            "./include/Rendering/Vulkan/VulkanShaderData.h",
            "./include/Rendering/Vulkan/VulkanShaderStorageObject.h",
            "./include/Rendering/Vulkan/VulkanSwapchain.h",
            "./include/Rendering/Vulkan/VulkanTexture.h",
            "./include/Rendering/Vulkan/VulkanTextureSampler.h",
            "./include/Rendering/Vulkan/VulkanUniformBuffer.h",
            "./include/Rendering/Vulkan/VulkanVideoTexture.h"
        );
    }

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");
    // TODO: Should probably make changes so we can also use "-Wextra" do not as we use list initializers for structs currently
    // It is one of the annoying things that was supported by C for ages and because of that all major compilers support it but took C++ a while to catch up therefore a warning
    CUBE_CProject_AppendCFlag(&project, "-Wall");
    if (a_targetPlatform == TargetPlatform_Linux)
    {
        // Should probably only turn this on with the Linux GCC version
        // This is the source of truth for us as it is the main compiler
        CUBE_CProject_AppendCFlag(&project, "-Werror");
    }

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        if (a_targetPlatform != TargetPlatform_Windows)
        {
            CUBE_CProject_AppendCFlag(&project, "-fsanitize=address");
            CUBE_CProject_AppendCFlag(&project, "-rdynamic");
        }

        if (a_targetPlatform == TargetPlatform_LinuxZig)
        {
            CUBE_CProject_AppendReference(&project, "asan");
        }

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-fwhole-program");

        if (a_targetPlatform == TargetPlatform_LinuxZig)
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86_64_v2");
        }
        else
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86-64-v2");
        }

        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        if (a_targetPlatform != TargetPlatform_Windows)
        {
            CUBE_CProject_AppendCFlag(&project, "-fsanitize=address");
        }

        if (a_targetPlatform == TargetPlatform_LinuxZig)
        {
            CUBE_CProject_AppendReference(&project, "asan");
        }

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-fwhole-program");

        if (a_targetPlatform == TargetPlatform_LinuxZig)
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86_64_v2");
        }
        else
        {
            CUBE_CProject_AppendCFlag(&project, "-march=x86-64-v2");
        }

        CUBE_CProject_AppendCFlag(&project, "-s");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        CUBE_CProject_AppendCFlag(&project, "-ffunction-sections");
        CUBE_CProject_AppendCFlag(&project, "-fdata-sections"); 

        CUBE_CProject_AppendCFlag(&project, "-Wl,--gc-sections");

        break;
    }
    }

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );

        CUBE_CProject_AppendIncludePaths(&project, "../deps/Mono/Windows/include");

        CUBE_CProject_AppendSource(&project, "./src/Library/LibXInput.cpp");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianCore/build/IcarianCore.lib",

            "../deps/glfw/build/GLFW.lib",
            "../deps/miniz/build/miniz.lib",
            "../deps/KTX-Software/build/c/ktxc.lib",
            "../deps/KTX-Software/build/cpp/ktxcpp.lib",
            "../deps/Mono/Windows/lib/mono-2.0-sgen.lib",
            "../deps/Mono/Windows/lib/MonoPosixHelper.lib",
            "../deps/zlib/build/zlib.lib",
            "../deps/assimp/build/assimp.lib",
            "../deps/assimp/contrib/unzip/build/unzip.lib",
            "../deps/enet/build/enet.lib",
            "../deps/meshoptimizer/build/meshoptimizer.lib",

            "./lib/glslang/build/glslang.lib",
            "./lib/glslang/build/SPIRV.lib",
            "./lib/glslang/External/spirv-tools/build/SPIRV-Tools.lib",
            "./lib/JoltPhysics/build/Jolt.lib"
        );

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");
        CUBE_CProject_AppendReference(&project, "winmm");
        CUBE_CProject_AppendReference(&project, "ole32");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        if (!a_flags.RemoteMode && a_configuration == BuildConfiguration_Release)
        {
            CUBE_CProject_AppendCFlag(&project, "-Wl,-subsystem,windows");
        }

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_CProject_AppendIncludePaths(&project, "../deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianCore/build/libIcarianCore.a",

            "../deps/glfw/build/libGLFW.a",
            "../deps/miniz/build/libminiz.a",
            "../deps/KTX-Software/build/c/libktxc.a",
            "../deps/KTX-Software/build/cpp/libktxcpp.a",
            "../deps/Mono/Linux/lib/libmonosgen-2.0.a",
            "../deps/zlib/build/libzlib.a",
            "../deps/assimp/build/libassimp.a",
            "../deps/assimp/contrib/unzip/build/libunzip.a",
            "../deps/enet/build/libenet.a",
            "../deps/meshoptimizer/build/libmeshoptimizer.a",

            "./lib/glslang/build/libglslang.a",
            "./lib/glslang/build/libSPIRV.a",
            "./lib/glslang/External/spirv-tools/build/libSPIRV-Tools.a",
            "./lib/JoltPhysics/build/libJolt.a"
        );

        CUBE_CProject_AppendReference(&project, "stdc++");
        CUBE_CProject_AppendReference(&project, "atomic");
        CUBE_CProject_AppendReference(&project, "m");

        break;
    }
    // Can we all agree FUCK CONTAINERS
    case TargetPlatform_LinuxSteam:
    {
        CUBE_CProject_AppendIncludePaths(&project, "../deps/Mono/LinuxSteam/include/mono-2.0");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianCore/build/libIcarianCore.a",

            "../deps/glfw/build/libGLFW.a",
            "../deps/miniz/build/libminiz.a",
            "../deps/KTX-Software/build/c/libktxc.a",
            "../deps/KTX-Software/build/cpp/libktxcpp.a",
            "../deps/Mono/LinuxSteam/lib/libmonosgen-2.0.a",
            "../deps/zlib/build/libzlib.a",
            "../deps/assimp/build/libassimp.a",
            "../deps/assimp/contrib/unzip/build/libunzip.a",
            "../deps/enet/build/libenet.a",
            "../deps/meshoptimizer/build/libmeshoptimizer.a",

            "./lib/glslang/build/libglslang.a",
            "./lib/glslang/build/libSPIRV.a",
            "./lib/glslang/External/spirv-tools/build/libSPIRV-Tools.a",
            "./lib/JoltPhysics/build/libJolt.a"
        );

        CUBE_CProject_AppendCFlag(&project, "-pthread");

        CUBE_CProject_AppendReference(&project, "dl");
        CUBE_CProject_AppendReference(&project, "rt");
        CUBE_CProject_AppendReference(&project, "stdc++");
        CUBE_CProject_AppendReference(&project, "atomic");
        CUBE_CProject_AppendReference(&project, "m");

        break;
    }
    }

    return project;
}

static DependencyProject* BuildIcarianNativeDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    return BuildIcarianNativeIDependencies(a_count, a_targetPlatform, a_configuration);
}

#ifdef __cplusplus
}
#endif

#endif
