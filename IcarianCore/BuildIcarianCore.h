#ifndef INCLUDED_HEADER_BUILDFLAREBASE
#define INCLUDED_HEADER_BUILDFLAREBASE

#include "CUBE/CUBE.h"

#include "../BuildBase.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CProject BuildIcarianCoreProject(CBBOOL a_enableAssert, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianCore");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_enableAssert)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIAN_ENABLE_ASSERT");
    }

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendDefines(&project, 
        "GLM_FORCE_QUAT_DATA_XYZW",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_FORCE_RADIANS"
    );

    if (a_targetPlatform == TargetPlatform_Windows)
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );
    }

    CUBE_CProject_AppendIncludePaths(&project, 
        "./include",

        "../deps/flare-glm",
        "../deps/flare-tinyxml2",
        "../deps/OpenFBX/src",
        "../deps/tinygltf",
        "../deps/enet/include",

        "../EngineInterop"
    );

    CUBE_CProject_AppendSources(&project,
        "../deps/flare-tinyxml2/tinyxml2.cpp",

        "./src/FlareShader.cpp",
        "./src/InputBindings.cpp",
        "./src/IPCPipe.cpp",
        "./src/MonoNativeImpl.cpp",
        "./src/SocketPipe.cpp"
    );

    CUBE_CProject_AppendRebuildSources(&project,
        "./include/Core/Bitfield.h",
        "./include/Core/CommunicationPipe.h",
        "./include/Core/CRC.h",
        "./include/Core/DMASwapBuffer.h",
        "./include/Core/Endian.h",
        "./include/Core/FlareShader.h",
        "./include/Core/IcarianAssert.h",
        "./include/Core/IcarianDefer.h",
        "./include/Core/IcarianError.h",
        "./include/Core/IcarianLambda.h",
        "./include/Core/IcarianPragma.h",
        "./include/Core/InputBindings.h",
        "./include/Core/IPCPipe.h",
        "./include/Core/MonoNativeImpl.h",
        "./include/Core/Pipefile.h",
        "./include/Core/PipeMessage.h",
        "./include/Core/ShaderBuffers.h",
        "./include/Core/SharedMemoryBuffer.h",
        "./include/Core/SocketPipe.h",
        "./include/Core/StringUtils.h",
        "./include/Core/WindowsHeaders.h"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c++17");
    CUBE_CProject_AppendCFlag(&project, "-Wall");
    if (a_targetPlatform == TargetPlatform_Linux)
    {
        // Should probably only have 1 as a source of truth
        // Can still spit out warning but on other compilers but GCC will be the one trusted when they contradict
        CUBE_CProject_AppendCFlag(&project, "-Werror");
    }

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );

        CUBE_CProject_AppendIncludePath(&project, "../deps/Mono/Windows/include");

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_CProject_AppendIncludePath(&project, "../deps/Mono/Linux/include/mono-2.0");

        break;
    }
    case TargetPlatform_LinuxSteam:
    {
        CUBE_CProject_AppendIncludePath(&project, "../deps/Mono/LinuxSteam/include/mono-2.0");

        break;
    }
    }

    switch (a_configuration)
    {
    case BuildConfiguration_Debug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");
        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    case BuildConfiguration_Release:
    {
        CUBE_CProject_AppendCFlag(&project, "-mavx");
        // CUBE_CProject_AppendCFlag(&project, "-mavx2");
        CUBE_CProject_AppendCFlag(&project, "-msse4.2");

        CUBE_CProject_AppendCFlag(&project, "-O3");
        CUBE_CProject_AppendCFlag(&project, "-flto=auto");
        CUBE_CProject_AppendCFlag(&project, "-ffat-lto-objects");

        break;
    }
    }

    return project;
}

#ifdef __cplusplus
}
#endif

#endif
