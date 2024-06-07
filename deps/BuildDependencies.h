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
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
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

// Not all platforms have a miniz implementation, so we need to build it ourselves for OpenFBX and KTX
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

CUBE_CProject BuildKTXC(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("ktxc");
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

    if (a_targetPlatform == TargetPlatform_Windows)
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );
    }

    CUBE_CProject_AppendDefines(&project, 
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2",
        "BASISU_SUPPORT_OPENCL=0",
        "BASISU_SUPPORT_SSE=1",
        "KTX_FEATURE_WRITE=0"
    );

    CUBE_CProject_AppendIncludePaths(&project, 
        ".",
        "../gen/KTX-Software/",
        "./include/",
        "./utils/",
        "./external"
    );

    CUBE_CProject_AppendSources(&project, 
        "./lib/checkheader.c",
        "./lib/filestream.c",
        "./lib/hashlist.c",
        "./lib/info.c",
        "./lib/memstream.c",
        "./lib/strings.c",
        "./lib/swap.c",
        "./lib/texture.c",
        "./lib/texture1.c",
        "./lib/texture2.c",
        "./lib/vkformat_check.c",
        "./lib/vkformat_str.c",
        "./lib/vkformat_typesize.c",

        "./external/basisu/zstd/zstd.c",
        "./external/dfdutils/createdfd.c",
        "./external/dfdutils/colourspaces.c",
        "./external/dfdutils/interpretdfd.c",
        "./external/dfdutils/printdfd.c",
        "./external/dfdutils/queries.c",
        "./external/dfdutils/vk2dfd.c"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c11");

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
CUBE_CProject BuildKTXCPP(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("ktxcpp");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    if (a_targetPlatform == TargetPlatform_Windows)
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );
    }

    CUBE_CProject_AppendDefines(&project, 
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2",
        "BASISU_SUPPORT_OPENCL=0",
        "BASISU_SUPPORT_SSE=1",
        "KTX_FEATURE_WRITE=0"
    );

    CUBE_CProject_AppendIncludePaths(&project, 
        ".",
        "../gen/KTX-Software/",
        "./include/",
        "./utils/",
        "./external"
    );

    CUBE_CProject_AppendSources(&project, 
        "./lib/basis_transcode.cpp",
        "./lib/miniz_wrapper.cpp",
        // "./lib/etcdec.cxx",
        "./lib/etcunpack.cxx",

        "./external/basisu/transcoder/basisu_transcoder.cpp"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c++11");
    CUBE_CProject_AppendCFlag(&project, "-msse4.1");

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
CUBE_CProject BuildKTXWriteC(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("ktxwritec");
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

    if (a_targetPlatform == TargetPlatform_Windows)
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );
    }

    CUBE_CProject_AppendDefines(&project, 
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2",
        "BASISU_SUPPORT_OPENCL=0",
        "BASISU_SUPPORT_SSE=1",
        "KTX_FEATURE_WRITE=1"
    );

    CUBE_CProject_AppendIncludePaths(&project, 
        ".",
        "../gen/KTX-Software/",
        "./include/",
        "./utils/",
        "./external"
    );

    CUBE_CProject_AppendSources(&project, 
        "./lib/checkheader.c",
        "./lib/filestream.c",
        "./lib/hashlist.c",
        "./lib/info.c",
        "./lib/memstream.c",
        "./lib/strings.c",
        "./lib/swap.c",
        "./lib/texture.c",
        "./lib/texture1.c",
        "./lib/texture2.c",
        "./lib/vkformat_check.c",
        "./lib/vkformat_str.c",
        "./lib/vkformat_typesize.c",
        "./lib/writer1.c",
        "./lib/writer2.c",

        "./external/basisu/zstd/zstd.c",
        "./external/dfdutils/createdfd.c",
        "./external/dfdutils/colourspaces.c",
        "./external/dfdutils/interpretdfd.c",
        "./external/dfdutils/printdfd.c",
        "./external/dfdutils/queries.c",
        "./external/dfdutils/vk2dfd.c"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c11");

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
CUBE_CProject BuildKTXWriteCPP(e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    CUBE_CProject project = { 0 };
    project.Name = CUBE_StackString_CreateC("ktxwritecpp");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_configuration == BuildConfiguration_Debug)
    {
        CUBE_CProject_AppendDefine(&project, "DEBUG");
    }
    else 
    {
        CUBE_CProject_AppendDefine(&project, "NDEBUG");
    }

    if (a_targetPlatform == TargetPlatform_Windows)
    {
        CUBE_CProject_AppendDefines(&project, 
            "WIN32",
            "_WIN32"
        );
    }

    CUBE_CProject_AppendDefines(&project, 
        "KHRONOS_STATIC",
        "LIBKTX",
        "KTX_FEATURE_KTX1",
        "KTX_FEATURE_KTX2",
        "BASISU_SUPPORT_OPENCL=0",
        "BASISU_SUPPORT_SSE=1",
        "KTX_FEATURE_WRITE=1"
    );

    CUBE_CProject_AppendIncludePaths(&project, 
        ".",
        "../gen/KTX-Software/",
        "./include/",
        "./utils/",
        "./external/"
    );

    CUBE_CProject_AppendSources(&project, 
        "./lib/basis_encode.cpp",
        "./lib/basis_transcode.cpp",
        "./lib/miniz_wrapper.cpp",
        // "./lib/etcdec.cxx",
        // "./lib/etcunpack.cxx"

        "./external/basisu/encoder/basisu_backend.cpp",
        "./external/basisu/encoder/basisu_basis_file.cpp",
        "./external/basisu/encoder/basisu_bc7enc.cpp",
        "./external/basisu/encoder/basisu_comp.cpp",
        "./external/basisu/encoder/basisu_enc.cpp",
        "./external/basisu/encoder/basisu_etc.cpp",
        "./external/basisu/encoder/basisu_frontend.cpp",
        "./external/basisu/encoder/basisu_gpu_texture.cpp",
        "./external/basisu/encoder/basisu_kernels_sse.cpp",
        "./external/basisu/encoder/basisu_opencl.cpp",
        "./external/basisu/encoder/basisu_pvrtc1_4.cpp",
        "./external/basisu/encoder/basisu_resample_filters.cpp",
        "./external/basisu/encoder/basisu_resampler.cpp",
        "./external/basisu/encoder/basisu_ssim.cpp",
        "./external/basisu/encoder/basisu_uastc_enc.cpp",

        "./external/basisu/transcoder/basisu_transcoder.cpp"
    );

    CUBE_CProject_AppendCFlag(&project, "-std=c++11");
    CUBE_CProject_AppendCFlag(&project, "-msse4.1");

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
    *a_count = 7;

    DependencyProject* projects = (DependencyProject*)malloc(sizeof(DependencyProject) * (*a_count));

    projects[0].Project = BuildGLFW(a_targetPlatform, a_configuration);
    projects[0].WorkingDirectory = "deps/flare-glfw";
    projects[0].Export = CBTRUE;

    // I am glad the seperated the OpenGL code from the rest of the library but the Vulkan code is tied in deeply and they use a mix of C and C++ ahhhhhhh................
    // Fucking pick one language and use it please otherwise it turns into a fucking mess
    // The most frustrating part is in some of the C++ files they just extern to C FFS
    // This library feels like a hack job and needs to be cleaned up
    // May consider down the line
    // Yes I am aware some of it is my build system is shit but still
    projects[1].Project = BuildKTXC(a_targetPlatform, a_configuration);
    projects[1].WorkingDirectory = "deps/KTX-Software";
    projects[1].Export = CBTRUE;

    projects[2].Project = BuildKTXCPP(a_targetPlatform, a_configuration);
    projects[2].WorkingDirectory = "deps/KTX-Software";
    projects[2].Export = CBTRUE;

    projects[3].Project = BuildKTXWriteC(a_targetPlatform, a_configuration);
    projects[3].WorkingDirectory = "deps/KTX-Software";
    projects[3].Export = CBFALSE;

    projects[4].Project = BuildKTXWriteCPP(a_targetPlatform, a_configuration);
    projects[4].WorkingDirectory = "deps/KTX-Software";
    projects[4].Export = CBFALSE;

    projects[5].Project = BuildMINIZ(a_targetPlatform, a_configuration);
    projects[5].WorkingDirectory = "deps/miniz";
    projects[5].Export = CBTRUE;

    projects[6].Project = BuildOpenFBXLibDeflate(a_targetPlatform, a_configuration);
    projects[6].WorkingDirectory = "deps/OpenFBX";
    projects[6].Export = CBTRUE;

    return projects;
}

#ifdef __cplusplus
}
#endif

#endif