#ifndef INCLUDED_HEADER_BUILDICARIANNATIVE
#define INCLUDED_HEADER_BUILDICARIANNATIVE

#include "CUBE/CUBE.h"

#include "../BuildBase.h"
#include "lib/BuildIcarianNativeDependencies.h"

#ifdef __cplusplus
extern "C" {
#endif

const static char* IcarianNativeShaderBasePaths[] =
{
    "shaders/AmbientLight.fpix",
    "shaders/DirectionalLight.fpix",
    "shaders/PointLight.fpix",
    "shaders/Post.fpix",
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

CBBOOL WriteIcarianNativeShadersToHeader(const char* a_workingPath)
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

CUBE_CProject BuildIcarianNativeProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration, CBBOOL a_enableTrace, CBBOOL a_enableProfiler)
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
        "ICARIANNATIVE_VERSION_MINOR=0",
        "ICARIANNATIVE_VERSION_PATCH=0",
        commitDefine.Data,
        "ICARIANNATIVE_VERSION_TAG=DEV",

        "ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN",

        "GLM_FORCE_QUAT_DATA_XYZW",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_FORCE_RADIANS",
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
        "include",
        "../EngineInterop",
        "../IcarianCore/include",
        "../deps/flare-glfw/include",
        "../deps/flare-glm",
        "../deps/flare-stb",
        "../deps/KTX-Software/include",
        "../deps/flare-tinyxml2",
        "lib/enet/include",
        "lib/glslang",
        "lib/JoltPhysics",
        "lib/openal-soft/include",
        "lib/SPIRV-Tools/include",
        "lib/VulkanMemoryAllocator/include"
    );

    CUBE_CProject_AppendSource(&project, "../deps/flare-tinyxml2/tinyxml2.cpp");

    CUBE_CProject_AppendSource(&project, "src/AnimationController.cpp");
    CUBE_CProject_AppendSource(&project, "src/AnimationControllerBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/Application.cpp");
    CUBE_CProject_AppendSource(&project, "src/AudioEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/AudioEngineBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/Config.cpp");
    CUBE_CProject_AppendSource(&project, "src/DeletionQueue.cpp");
    CUBE_CProject_AppendSource(&project, "src/Font.cpp");
    CUBE_CProject_AppendSource(&project, "src/GamePad.cpp");
    CUBE_CProject_AppendSource(&project, "src/GLFWAppWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/HeadlessAppWindow.cpp");
    CUBE_CProject_AppendSource(&project, "src/IcBodyActivationListener.cpp");
    CUBE_CProject_AppendSource(&project, "src/IcBroadPhaseLayerInterface.cpp");
    CUBE_CProject_AppendSource(&project, "src/IcContactListener.cpp");
    CUBE_CProject_AppendSource(&project, "src/IcObjectLayerPairFilter.cpp");
    CUBE_CProject_AppendSource(&project, "src/IcObjectVsBroadPhaseLayerFilter.cpp");
    CUBE_CProject_AppendSource(&project, "src/IcPhysicsJobSystem.cpp");
    CUBE_CProject_AppendSource(&project, "src/ImageUIElement.cpp");
    CUBE_CProject_AppendSource(&project, "src/InputManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/Logger.cpp");
    CUBE_CProject_AppendSource(&project, "src/main.cpp");
    CUBE_CProject_AppendSource(&project, "src/MaterialRenderStack.cpp");
    CUBE_CProject_AppendSource(&project, "src/NetworkClient.cpp");
    CUBE_CProject_AppendSource(&project, "src/NetworkManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/NetworkServer.cpp");
    CUBE_CProject_AppendSource(&project, "src/NullRenderEngineBackend.cpp");
    CUBE_CProject_AppendSource(&project, "src/ObjectManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/OGGAudioClip.cpp");
    CUBE_CProject_AppendSource(&project, "src/PhysicsEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/PhysicsEngineBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/Profiler.cpp");
    CUBE_CProject_AppendSource(&project, "src/Random.cpp");
    CUBE_CProject_AppendSource(&project, "src/RenderAssetStore.cpp");
    CUBE_CProject_AppendSource(&project, "src/RenderAssetStoreBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/RenderEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeFunction.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeThreadJob.cpp");
    CUBE_CProject_AppendSource(&project, "src/Scribe.cpp");
    CUBE_CProject_AppendSource(&project, "src/ShaderTable.cpp");
    CUBE_CProject_AppendSource(&project, "src/SPIRVTools.cpp");
    CUBE_CProject_AppendSource(&project, "src/TextUIElement.cpp");
    CUBE_CProject_AppendSource(&project, "src/ThreadPool.cpp");
    CUBE_CProject_AppendSource(&project, "src/UIControl.cpp");
    CUBE_CProject_AppendSource(&project, "src/UIControlBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanComputeEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanComputeEngineBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanComputeLayout.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanComputeParticle.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanComputePipeline.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanComputeShader.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanDepthCubeRenderTexture.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanDepthRenderTexture.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanGraphicsEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanGraphicsEngineBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanGraphicsParticle2D.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanLightData.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanModel.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanParticleShaderGenerator.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanPipeline.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanPixelShader.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanPushPool.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanRenderCommand.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanRenderEngineBackend.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanRenderTexture.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanShaderData.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanShaderStorageObject.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanSwapchain.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanTexture.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanTextureSampler.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanUniformBuffer.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanVertexShader.cpp");
    CUBE_CProject_AppendSource(&project, "src/WAVAudioClip.cpp");

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

        CUBE_CProject_AppendCFlag(&project, "-O3");

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

            "../deps/flare-glfw/build/GLFW.lib",
            "../deps/miniz/build/miniz.lib",
            "../deps/KTX-Software/build/ktxc.lib",
            "../deps/KTX-Software/build/ktxcpp.lib",
            "../deps/Mono/Windows/lib/mono-2.0-sgen.lib",
            "../deps/Mono/Windows/lib/MonoPosixHelper.lib",
            "../deps/OpenFBX/build/OpenFBXLibDeflate.lib",

            "lib/enet/build/enet.lib",
            "lib/glslang/build/glslang.lib",
            "lib/glslang/build/SPIRV.lib",
            "lib/JoltPhysics/build/Jolt.lib",
            "lib/openal-soft/build/OpenALSoft.lib",
            "lib/SPIRV-Tools/build/SPIRV-Tools.lib"
        );

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "vulkan-1");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");
        CUBE_CProject_AppendReference(&project, "winmm");
        CUBE_CProject_AppendReference(&project, "ole32");
        CUBE_CProject_AppendReference(&project, "xinput");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibraries(&project,
            "../IcarianCore/build/libIcarianCore.a",

            "../deps/flare-glfw/build/libGLFW.a",
            "../deps/miniz/build/libminiz.a",
            "../deps/KTX-Software/build/libktxc.a",
            "../deps/KTX-Software/build/libktxcpp.a",
            "../deps/Mono/Linux/lib/libmonosgen-2.0.a",
            "../deps/OpenFBX/build/libOpenFBXLibDeflate.a",

            "lib/enet/build/libenet.a",
            "lib/glslang/build/libglslang.a",
            "lib/glslang/build/libSPIRV.a",
            "lib/JoltPhysics/build/libJolt.a",
            "lib/openal-soft/build/libOpenALSoft.a",
            "lib/SPIRV-Tools/build/libSPIRV-Tools.a"
        );

        CUBE_CProject_AppendReference(&project, "vulkan");
        CUBE_CProject_AppendReference(&project, "z");

        CUBE_CProject_AppendReference(&project, "stdc++");
        CUBE_CProject_AppendReference(&project, "atomic");
        CUBE_CProject_AppendReference(&project, "m");

        break;
    }
    }

    return project;
}

DependencyProject* BuildIcarianNativeDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    return BuildIcarianNativeIDependencies(a_count, a_targetPlatform, a_configuration);
}

#ifdef __cplusplus
}
#endif

#endif