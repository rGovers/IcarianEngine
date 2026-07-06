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

static CUBE_CProject BuildIcarianNativeProject(const char* a_path, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration, IcarianNativeProjectFlags a_flags)
{
    CUBE_Path path = CUBE_Path_CreateC(a_path);

    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianNative");
    project.Target = CUBE_CProjectTarget_Exe;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build");

    if (a_configuration == BuildConfiguration_Debug || a_configuration == BuildConfiguration_DebugFast)
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
        "ICARIANNATIVE_VERSION_MAJOR=2026",
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
        "../deps/flare-tinyxml2/tinyxml2.cpp"
    );

    CUBE_Path srcPrefix = CUBE_Path_CreateC("src");
    CUBE_Path srcPath = CUBE_Path_CombineP(&path, &srcPrefix);

    CUBE_CProject_AppendSourceDirectoryP(&project, &srcPath, &srcPrefix, CBFALSE);

    CUBE_Path_Destroy(&srcPath);
    CUBE_Path_Destroy(&srcPrefix);

    CUBE_Path coreIncludePrefix = CUBE_Path_CreateC("../IcarianCore/include/Core");
    CUBE_Path coreIncludePath = CUBE_Path_CombineP(&path, &coreIncludePrefix);

    CUBE_CProject_AppendRebuildSourceDirectoryP(&project, &coreIncludePath, &coreIncludePrefix, CBFALSE);

    CUBE_Path_Destroy(&coreIncludePath);
    CUBE_Path_Destroy(&coreIncludePrefix);

    CUBE_Path interopIncludePrefix = CUBE_Path_CreateC("../EngineInterop");
    CUBE_Path interopIncludePath = CUBE_Path_CombineP(&path, &interopIncludePrefix);

    CUBE_CProject_AppendRebuildSourceDirectoryP(&project, &interopIncludePath, &interopIncludePrefix, CBFALSE);

    CUBE_Path_Destroy(&interopIncludePath);
    CUBE_Path_Destroy(&interopIncludePrefix);

    CUBE_Path includePrefix = CUBE_Path_CreateC("include");
    CUBE_Path includePath = CUBE_Path_CombineP(&path, &includePrefix);

    // Will catch some files not always in the build but doing recursive because lazy and not the end of the world if it triggers a rebuild when modified
    CUBE_CProject_AppendRebuildSourceDirectoryP(&project, &includePath, &includePrefix, CBTRUE);

    CUBE_Path_Destroy(&includePath);
    CUBE_Path_Destroy(&includePrefix);

    CUBE_Path shaderPrefix = CUBE_Path_CreateC("shaders");
    CUBE_Path shaderPath = CUBE_Path_CombineP(&path, &shaderPrefix);

    CUBE_CProject_AppendRebuildSourceDirectoryP(&project, &shaderPath, &shaderPrefix, CBFALSE);

    CUBE_Path_Destroy(&shaderPath);
    CUBE_Path_Destroy(&shaderPrefix);

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

        CUBE_CProject_AppendSource(&project, "./src/Library/LibVulkan.cpp");

        CUBE_Path vulkanSrcPrefix = CUBE_Path_CreateC("src/Platform/Vulkan");
        CUBE_Path vulkanSrcPath = CUBE_Path_CombineP(&path, &vulkanSrcPrefix);

        CUBE_CProject_AppendSourceDirectoryP(&project, &vulkanSrcPath, &vulkanSrcPrefix, CBFALSE);

        CUBE_Path_Destroy(&vulkanSrcPrefix);
        CUBE_Path_Destroy(&vulkanSrcPath);
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
    case BuildConfiguration_DebugFast:
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_FAST_ALLOCATOR");
        CUBE_CProject_AppendCFlag(&project, "-g");

        if (a_targetPlatform != TargetPlatform_Windows)
        {
            CUBE_CProject_AppendCFlag(&project, "-rdynamic");
        }

        if (a_targetPlatform == TargetPlatform_Linux)
        {
            CUBE_CProject_AppendCFlag(&project, "-Og");
        }

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_FAST_ALLOCATOR");
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
        CUBE_CProject_AppendDefine(&project, "ICARIANNATIVE_FAST_ALLOCATOR");
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

    CUBE_Path_Destroy(&path);

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
