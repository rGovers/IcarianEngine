#ifndef INCLUDED_HEADER_BUILDDEPENDENCIES
#define INCLUDED_HEADER_BUILDDEPENDENCIES

#include "CUBE/CUBE.h"

#include <stdlib.h>

#include "../BuildBase.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    CUBE_CProject Project;
    const char* WorkingDirectory;
} DependencyProject;

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

    return project;
}

DependencyProject* BuildDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    DependencyProject* projects = (DependencyProject*)malloc(sizeof(DependencyProject) * 1);

    projects[0].Project = BuildGLFW(a_targetPlatform, a_configuration);
    projects[0].WorkingDirectory = "deps/flare-glfw";

    *a_count = 1;

    return projects;
}

#ifdef __cplusplus
}
#endif

#endif