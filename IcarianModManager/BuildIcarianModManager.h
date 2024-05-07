#ifndef INCLUDED_HEADER_BUILDICARIANMODMANAGER
#define INCLUDED_HEADER_BUILDICARIANMODMANAGER

#ifdef __cplusplus
extern "C" {
#endif

#include "CUBE/CUBE.h"

#include "../BuildBase.h"

static CUBE_CProject BuildIcarianModManagerProject(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("IcarianModManager");
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

    CUBE_String commitDefine = CUBE_String_CreateC("ICARIANMODMANAGER_COMMIT_HASH=");
    CUBE_String_AppendSS(&commitDefine, &commitHash);

    CUBE_CProject_AppendDefines(&project,
        "ICARIANMODMANAGER_VERSION_MAJOR=2024",
        "ICARIANMODMANAGER_VERSION_MINOR=0",
        "ICARIANMODMANAGER_VERSION_PATCH=0",
        commitDefine.Data,
        "ICARIANMODMANAGER_VERSION_TAG=DEV"
    );

    CUBE_String_Destroy(&commitDefine);

    CUBE_CProject_AppendIncludePaths(&project, 
        "include",

        "../IcarianCore/include",

        "../deps/imgui",
        "../deps/flare-glfw/include",
        "../deps/flare-tinyxml2",
        "../deps/glad/include"
    );

    CUBE_CProject_AppendSources(&project,
        "../deps/flare-tinyxml2/tinyxml2.cpp",
        "../deps/imgui/imgui.cpp",
        "../deps/imgui/imgui_draw.cpp",
        "../deps/imgui/imgui_tables.cpp",
        "../deps/imgui/imgui_widgets.cpp",
        "../deps/imgui/backends/imgui_impl_glfw.cpp",
        "../deps/imgui/backends/imgui_impl_opengl3.cpp",
        "../deps/glad/src/glad.c",

        "src/Application.cpp",
        "src/main.cpp",
        "src/ModInfo.cpp",
        "src/ModManager.cpp"
    );

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

        break;
    }
    case BuildConfiguration_ReleaseWithDebug:
    {
        CUBE_CProject_AppendCFlag(&project, "-g");
        CUBE_CProject_AppendCFlag(&project, "-O3");

        if (a_targetPlatform != TargetPlatform_Windows)
        {
            CUBE_CProject_AppendCFlag(&project, "-fsanitize=address");
        }

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
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );

        CUBE_CProject_AppendLibraries(&project, 
            "../IcarianCore/build/IcarianCore.lib",

            "../deps/flare-glfw/build/GLFW.lib"
        );

        CUBE_CProject_AppendReference(&project, "gdi32");

        CUBE_CProject_AppendCFlag(&project, "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic");

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_CProject_AppendLibraries(&project, 
            "../IcarianCore/build/libIcarianCore.a",

            "../deps/flare-glfw/build/libGLFW.a"
        );

        CUBE_CProject_AppendReference(&project, "stdc++");
        CUBE_CProject_AppendReference(&project, "m");

        break;
    }
    }

    return project;
}

#ifdef  __cplusplus
}
#endif

#endif