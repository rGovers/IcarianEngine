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
    "shaders/DirectionalLight.fpix",
    "shaders/PointLight.fpix",
    "shaders/Post.fpix",
    "shaders/Quad.vert",
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

CUBE_CProject BuildIcarianNativeProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
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

    CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_VERSION_MAJOR=0");
    CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_VERSION_MINOR=1");
    CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN");

    CUBE_CProject_AppendIncludePath(&project, "include");
    CUBE_CProject_AppendIncludePath(&project, "../FlareBase/include");
    CUBE_CProject_AppendIncludePath(&project, "../deps/flare-glfw/include");
    CUBE_CProject_AppendIncludePath(&project, "../deps/flare-glm");
    CUBE_CProject_AppendIncludePath(&project, "../deps/flare-stb");
    CUBE_CProject_AppendIncludePath(&project, "../deps/flare-tinyxml2");
    CUBE_CProject_AppendIncludePath(&project, "lib/glslang");
    CUBE_CProject_AppendIncludePath(&project, "lib/JoltPhysics");
    CUBE_CProject_AppendIncludePath(&project, "lib/VulkanMemoryAllocator/include");

    CUBE_CProject_AppendSource(&project, "../deps/flare-tinyxml2/tinyxml2.cpp");

    CUBE_CProject_AppendSource(&project, "src/AnimationController.cpp");
    CUBE_CProject_AppendSource(&project, "src/AnimationControllerBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/Application.cpp");
    CUBE_CProject_AppendSource(&project, "src/Config.cpp");
    CUBE_CProject_AppendSource(&project, "src/Font.cpp");
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
    CUBE_CProject_AppendSource(&project, "src/NullRenderEngineBackend.cpp");
    CUBE_CProject_AppendSource(&project, "src/ObjectManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/PhysicsEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/PhysicsEngineBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/Profiler.cpp");
    CUBE_CProject_AppendSource(&project, "src/RenderEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeFunction.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeManager.cpp");
    CUBE_CProject_AppendSource(&project, "src/RuntimeThreadJob.cpp");
    CUBE_CProject_AppendSource(&project, "src/Scribe.cpp");
    CUBE_CProject_AppendSource(&project, "src/SpirvTools.cpp");
    CUBE_CProject_AppendSource(&project, "src/TextUIElement.cpp");
    CUBE_CProject_AppendSource(&project, "src/ThreadPool.cpp");
    CUBE_CProject_AppendSource(&project, "src/UIControl.cpp");
    CUBE_CProject_AppendSource(&project, "src/UIControlBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanDepthRenderTexture.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanGraphicsEngine.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanGraphicsEngineBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanLightData.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanModel.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanPipeline.cpp");
    CUBE_CProject_AppendSource(&project, "src/VulkanPixelShader.cpp");
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

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-O3");

        break;
    }
    }

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../deps/Mono/Windows/include");

        CUBE_CProject_AppendLibrary(&project, "../FlareBase/build/FlareBase.lib");

        CUBE_CProject_AppendLibrary(&project, "../deps/flare-glfw/build/GLFW.lib");
        CUBE_CProject_AppendLibrary(&project, "../deps/Mono/Windows/lib/mono-2.0-sgen.lib");
        CUBE_CProject_AppendLibrary(&project, "../deps/Mono/Windows/lib/MonoPosixHelper.lib");

        CUBE_CProject_AppendLibrary(&project, "lib/glslang/build/glslang.lib");
        CUBE_CProject_AppendLibrary(&project, "lib/glslang/build/OGLCompiler.lib");
        CUBE_CProject_AppendLibrary(&project, "lib/glslang/build/SPIRV.lib");
        CUBE_CProject_AppendLibrary(&project, "lib/JoltPhysics/build/Jolt.lib");

        CUBE_CProject_AppendReference(&project, "gdi32");
        CUBE_CProject_AppendReference(&project, "vulkan-1");
        CUBE_CProject_AppendReference(&project, "wsock32");
        CUBE_CProject_AppendReference(&project, "ws2_32");

        // Magic string to get std library to link with MinGW
        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        break;
    }
    case TargetPlatform_Linux:
    {
        CUBE_CProject_AppendSystemIncludePath(&project, "../deps/Mono/Linux/include/mono-2.0");

        CUBE_CProject_AppendLibrary(&project, "../FlareBase/build/libFlareBase.a");

        CUBE_CProject_AppendLibrary(&project, "../deps/flare-glfw/build/libGLFW.a");
        CUBE_CProject_AppendLibrary(&project, "../deps/Mono/Linux/lib/libmonosgen-2.0.a");

        CUBE_CProject_AppendLibrary(&project, "lib/glslang/build/libglslang.a");
        CUBE_CProject_AppendLibrary(&project, "lib/glslang/build/libOGLCompiler.a");
        CUBE_CProject_AppendLibrary(&project, "lib/glslang/build/libSPIRV.a");
        CUBE_CProject_AppendLibrary(&project, "lib/JoltPhysics/build/libJolt.a");

        CUBE_CProject_AppendReference(&project, "vulkan");
        CUBE_CProject_AppendReference(&project, "z");

        CUBE_CProject_AppendReference(&project, "stdc++");
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