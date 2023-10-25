#ifndef INCLUDED_HEADER_BUILDDEPENDENCIES
#define INCLUDED_HEADER_BUILDDEPENDENCIES

#include "CUBE/CUBE.h"

#include <stdlib.h>

#include "../BuildBase.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CProject BuildGLFW(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("GLFW");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_C;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendIncludePath(&project, "include");
    CUBE_CProject_AppendIncludePath(&project, "src");

    CUBE_CProject_AppendSource(&project, "src/context.c");
    CUBE_CProject_AppendSource(&project, "src/init.c");
    CUBE_CProject_AppendSource(&project, "src/input.c");
    CUBE_CProject_AppendSource(&project, "src/monitor.c");
    CUBE_CProject_AppendSource(&project, "src/platform.c");
    CUBE_CProject_AppendSource(&project, "src/vulkan.c");
    CUBE_CProject_AppendSource(&project, "src/window.c");
    CUBE_CProject_AppendSource(&project, "src/egl_context.c");
    CUBE_CProject_AppendSource(&project, "src/osmesa_context.c");
    CUBE_CProject_AppendSource(&project, "src/null_init.c");
    CUBE_CProject_AppendSource(&project, "src/null_monitor.c");
    CUBE_CProject_AppendSource(&project, "src/null_window.c");
    CUBE_CProject_AppendSource(&project, "src/null_joystick.c");

    switch (a_targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_CProject_AppendDefine(&project, "WIN32");
        CUBE_CProject_AppendDefine(&project, "_GLFW_WIN32");

        CUBE_CProject_AppendSource(&project, "src/wgl_context.c");
        CUBE_CProject_AppendSource(&project, "src/win32_init.c");
        CUBE_CProject_AppendSource(&project, "src/win32_joystick.c");
        CUBE_CProject_AppendSource(&project, "src/win32_module.c");
        CUBE_CProject_AppendSource(&project, "src/win32_monitor.c");
        CUBE_CProject_AppendSource(&project, "src/win32_thread.c");
        CUBE_CProject_AppendSource(&project, "src/win32_time.c");
        CUBE_CProject_AppendSource(&project, "src/win32_window.c");

        break;
    }
    case TargetPlatform_Linux:
    {
        CUBE_CProject_AppendDefine(&project, "_GLFW_X11");
        
        CUBE_CProject_AppendSource(&project, "src/glx_context.c");
        CUBE_CProject_AppendSource(&project, "src/linux_joystick.c");
        CUBE_CProject_AppendSource(&project, "src/posix_poll.c");
        CUBE_CProject_AppendSource(&project, "src/posix_module.c");
        CUBE_CProject_AppendSource(&project, "src/posix_time.c");
        CUBE_CProject_AppendSource(&project, "src/posix_thread.c");
        CUBE_CProject_AppendSource(&project, "src/x11_init.c");
        CUBE_CProject_AppendSource(&project, "src/x11_monitor.c");
        CUBE_CProject_AppendSource(&project, "src/x11_window.c");
        CUBE_CProject_AppendSource(&project, "src/xkb_unicode.c");

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

    return project;
}

// Not all platforms have a miniz implementation, so we need to build it ourselves for OpenFBX
CUBE_CProject BuildMINIZ(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("miniz");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_C;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendIncludePath(&project, ".");
    CUBE_CProject_AppendIncludePath(&project, "../gen/miniz");

    CUBE_CProject_AppendSource(&project, "./miniz_tdef.c");
    CUBE_CProject_AppendSource(&project, "./miniz_tinfl.c");
    CUBE_CProject_AppendSource(&project, "./miniz_zip.c");
    CUBE_CProject_AppendSource(&project, "./miniz.c");

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

    return project;
}

CUBE_CProject BuildOpenFBXLibDeflate(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("OpenFBXLibDeflate");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_C;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    CUBE_CProject_AppendIncludePath(&project, "src");

    CUBE_CProject_AppendSource(&project, "src/libdeflate.c");

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

    return project;
}

DependencyProject* BuildDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    *a_count = 3;

    DependencyProject* projects = (DependencyProject*)malloc(sizeof(DependencyProject) * (*a_count));

    projects[0].Project = BuildGLFW(a_targetPlatform, a_configuration);
    projects[0].WorkingDirectory = "deps/flare-glfw";

    projects[1].Project = BuildMINIZ(a_targetPlatform, a_configuration);
    projects[1].WorkingDirectory = "deps/miniz";

    projects[2].Project = BuildOpenFBXLibDeflate(a_targetPlatform, a_configuration);
    projects[2].WorkingDirectory = "deps/OpenFBX";

    return projects;
}

#ifdef __cplusplus
}
#endif

#endif