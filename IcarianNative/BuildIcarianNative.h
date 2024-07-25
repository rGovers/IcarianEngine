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
    "shaders/AmbientOcclusion.fpix",
    "shaders/AmbientOcclusionFilter.fpix",
    "shaders/AmbientLight.fpix",
    "shaders/Blend.fpix",
    "shaders/DirectionalLight.fpix",
    "shaders/PointLight.fpix",
    "shaders/PostAtmosphere.fpix",
    "shaders/PostEmission.fpix",
    "shaders/PostToneMap.fpix",
    "shaders/Quad.vert",
    "shaders/ShadowDirectionalLight.fpix",
    "shaders/ShadowPointLight.fpix",
    "shaders/ShadowSpotLight.fpix",
    "shaders/SpotLight.fpix",
    "shaders/UI.fvert",
    "shaders/UIImage.fpix",
    "shaders/UIText.fpix"
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

static CUBE_CProject BuildIcarianNativeProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration, CBBOOL a_enableTrace, CBBOOL a_enableProfiler)
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

    CUBE_StackString commitHash = CUBE_Git_GetCommitHashShort();

    CUBE_String commitDefine = CUBE_String_CreateC("ICARIANNATIVE_COMMIT_HASH=");
    CUBE_String_AppendSS(&commitDefine, &commitHash);

    CUBE_CProject_AppendDefines(&project,
        "ICARIANNATIVE_VERSION_MAJOR=2024",
        "ICARIANNATIVE_VERSION_MINOR=1",
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

    if (a_enableTrace)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_TRACE");
    }
    if (a_enableProfiler)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_PROFILER");
    }

    CUBE_CProject_AppendIncludePaths(&project, 
        "./include",
        "../EngineInterop",
        "../IcarianCore/include",
        "../deps/assimp/include",
        "../deps/gen/assimp",
        "../deps/glfw/include",
        "../deps/flare-glm",
        "../deps/flare-stb",
        "../deps/KTX-Software/include",
        "../deps/flare-tinyxml2",
        "./lib/enet/include",
        "./lib/glslang",
        "./lib/glslang/External/spirv-tools/include",
        "./lib/JoltPhysics",
        "./lib/minimp4",
        "./lib/openal-soft/include",
        "./lib/SPIRV-Tools/include",
        "./lib/VulkanMemoryAllocator/include"
    );

    CUBE_CProject_AppendSources(&project, 
        "../deps/flare-tinyxml2/tinyxml2.cpp",

        "./src/AnimationController.cpp",
        "./src/AnimationControllerBindings.cpp",
        "./src/Application.cpp",
        "./src/AudioEngine.cpp",
        "./src/AudioEngineBindings.cpp",
        "./src/Config.cpp",
        "./src/DeletionQueue.cpp",
        "./src/FileCache.cpp",
        "./src/Font.cpp",
        "./src/GamePad.cpp",
        "./src/GLFWAppWindow.cpp",
        "./src/H264.cpp",
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
        "./src/main.cpp",
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
        "./src/Profiler.cpp",
        "./src/Random.cpp",
        "./src/RenderAssetStore.cpp",
        "./src/RenderAssetStoreBindings.cpp",
        "./src/RenderEngine.cpp",
        "./src/RuntimeFunction.cpp",
        "./src/RuntimeManager.cpp",
        "./src/RuntimeThreadJob.cpp",
        "./src/Scribe.cpp",
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
            "./src/Platform/Vulkan/VulkanTexture.cpp",
            "./src/Platform/Vulkan/VulkanTextureSampler.cpp",
            "./src/Platform/Vulkan/VulkanUniformBuffer.cpp",
            "./src/Platform/Vulkan/VulkanVertexShader.cpp",
            "./src/Platform/Vulkan/VulkanVideoTexture.cpp",

            "./src/Library/LibVulkan.cpp"
        );
    }

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

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
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-flto");
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
        
        CUBE_CProject_AppendCFlag(&project, "-flto");
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

        CUBE_CProject_AppendSystemIncludePath(&project, "../deps/Mono/Windows/include");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianCore/build/IcarianCore.lib",

            "../deps/glfw/build/GLFW.lib",
            "../deps/miniz/build/miniz.lib",
            "../deps/KTX-Software/build/ktxc.lib",
            "../deps/KTX-Software/build/ktxcpp.lib",
            "../deps/Mono/Windows/lib/mono-2.0-sgen.lib",
            "../deps/Mono/Windows/lib/MonoPosixHelper.lib",
            "../deps/zlib/build/zlib.lib",
            "../deps/assimp/build/assimp.lib",
            "../deps/assimp/contrib/unzip/build/unzip.lib",

            "./lib/enet/build/enet.lib",
            "./lib/glslang/build/glslang.lib",
            "./lib/glslang/build/SPIRV.lib",
            "./lib/glslang/External/spirv-tools/build/SPIRV-Tools.lib",
            "./lib/JoltPhysics/build/Jolt.lib",
            "./lib/openal-soft/build/OpenALSoft.lib"
        );

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");
        CUBE_CProject_AppendReference(&project, "winmm");
        CUBE_CProject_AppendReference(&project, "ole32");
        CUBE_CProject_AppendReference(&project, "xinput");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        if (a_configuration == BuildConfiguration_Release)
        {
            CUBE_CProject_AppendCFlag(&project, "-Wl,-subsystem,windows");
        }

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianCore/build/libIcarianCore.a",

            "../deps/glfw/build/libGLFW.a",
            "../deps/miniz/build/libminiz.a",
            "../deps/KTX-Software/build/libktxc.a",
            "../deps/KTX-Software/build/libktxcpp.a",
            "../deps/Mono/Linux/lib/libmonosgen-2.0.a",
            "../deps/zlib/build/libzlib.a",
            "../deps/assimp/build/libassimp.a",
            "../deps/assimp/contrib/unzip/build/libunzip.a",

            "./lib/enet/build/libenet.a",
            "./lib/glslang/build/libglslang.a",
            "./lib/glslang/build/libSPIRV.a",
            "./lib/glslang/External/spirv-tools/build/libSPIRV-Tools.a",
            "./lib/JoltPhysics/build/libJolt.a",
            "./lib/openal-soft/build/libOpenALSoft.a"
        );

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