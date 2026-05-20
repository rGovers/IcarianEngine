#ifndef INCLUDED_HEADER_BUILDFLAREBASE
#define INCLUDED_HEADER_BUILDFLAREBASE

#include "CUBE/CUBE.h"

#include "../BuildBase.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CProject BuildIcarianCoreProject(const char* a_path, CBBOOL a_enableAssert, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_Path path = CUBE_Path_CreateC(a_path);

    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianCore");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_enableAssert)
    {
        CUBE_CProject_AppendDefine(&project, "ICARIAN_ENABLE_ASSERT");
    }

    if (a_configuration == BuildConfiguration_Debug || a_configuration == BuildConfiguration_DebugFast)
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
        "../deps/flare-tinyxml2/tinyxml2.cpp"
    );

    CUBE_Path srcPrefix = CUBE_Path_CreateC("src");
    CUBE_Path srcPath = CUBE_Path_CombineP(&path, &srcPrefix);

    CUBE_CProject_AppendSourceDirectoryP(&project, &srcPath, &srcPrefix, CBFALSE);

    CUBE_Path_Destroy(&srcPrefix);
    CUBE_Path_Destroy(&srcPath);

    CUBE_Path includePrefix = CUBE_Path_CreateC("include/Core");
    CUBE_Path includePath = CUBE_Path_CombineP(&path, &includePrefix);

    CUBE_CProject_AppendRebuildSourceDirectoryP(&project, &includePath, &includePrefix, CBFALSE);

    CUBE_Path_Destroy(&includePrefix);
    CUBE_Path_Destroy(&includePath);

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
    case BuildConfiguration_DebugFast:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");

        if (a_targetPlatform == TargetPlatform_Linux)
        {
            CUBE_CProject_AppendCFlag(&project, "-Og");
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

    CUBE_Path_Destroy(&path);

    return project;
}

#ifdef __cplusplus
}
#endif

#endif
