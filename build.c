#define CUBE_IMPLEMENTATION
#define CUBE_PRETTY_PRINT
#define CUBE_PRINT_COLOUR
// #define CUBE_PRINT_COMMANDS
#include "CUBE/CUBE.h"

#include <stdio.h>
#include <string.h>

#include "BuildBase.h"

#include "deps/BuildDependencies.h"
#include "IcarianCore/BuildIcarianCore.h"
#include "IcarianCS/BuildIcarianCS.h"
#include "IcarianModManager/BuildIcarianModManager.h"
#include "IcarianNative/BuildIcarianNative.h"

void PrintEngineHelp()
{
    PrintHelp();

    printf("Engine Arguments:\n");

    printf("  --enable-trace - Enables debug logging for the engine \n");
    printf("  --enable-profiler - Enables the internal profiler for the engine \n");
    printf("  --noprint - Disables extra print information during build \n");
}

static const char EnableTraceString[] = "--enable-trace";
static const CBUINT32 EnableTraceStringLen = sizeof(EnableTraceString) - 1;
static const char EnableProfilerString[] = "--enable-profiler";
static const CBUINT32 EnableProfilerStringLen = sizeof(EnableProfilerString) - 1;
static const char RemoteString[] = "--remote";
static const CBUINT32 RemoteStringLen = sizeof(RemoteString) - 1;
static const char ModString[] = "--disable-mod";
static const CBUINT32 ModStringLen = sizeof(ModString) - 1;
static const char NoPrintString[] = "--noprint";
static const CBUINT32 NoPrintStringLen = sizeof(NoPrintString) - 1;

int main(int a_argc, char** a_argv)
{
    e_TargetPlatform targetPlatform;
    e_BuildConfiguration buildConfiguration;

    CUBE_CProject icarianCoreProject;
    CUBE_CSProject icarianCSProject;
    CUBE_CProject icarianNativeProject;
    CUBE_CProject icarianModManagerProject;

    e_CUBE_CProjectCompiler compiler;

    CBUINT32 dependencyProjectCount;
    DependencyProject* dependencyProjects;

    CBUINT32 jobThreads;

    CBUINT32 lineCount;
    CUBE_String* lines;

    CBBOOL ret;
    CBBOOL rebuild;

    CBBOOL enableTrace;
    CBBOOL enableProfiler;
    CBBOOL enableMod;
    CBBOOL remoteMode;
    CBBOOL noPrint;

#ifdef _WIN32
    targetPlatform = TargetPlatform_Windows;
#else
    targetPlatform = TargetPlatform_Linux;
#endif

    buildConfiguration = BuildConfiguration_Debug;

    lineCount = 0;
    lines = CBNULL;

    jobThreads = 4;

    enableTrace = CBFALSE;
    enableProfiler = CBFALSE;
    enableMod = CBTRUE;
    remoteMode = CBFALSE;
    rebuild = CBFALSE;
    noPrint = CBFALSE;

    // Dont need the first arg
    for (int i = 1; i < a_argc; ++i)
    {
        if (strncmp(a_argv[i], PlatformString, PlatformStringLen) == 0)
        {
            const char* platformStr = a_argv[i] + PlatformStringLen;
            while (*platformStr != '=' && *platformStr != '\0')
            {
                ++platformStr;
            }

            if (*platformStr == '\0')
            {
                printf("Invalid platform argument: %s\n", a_argv[i]);

                return 1;
            }

            ++platformStr;

            if (strcmp(platformStr, "windows") == 0)
            {
                targetPlatform = TargetPlatform_Windows;
            }
            else if (strcmp(platformStr, "linux") == 0)
            {
                targetPlatform = TargetPlatform_Linux;
            }
            else if (strcmp(platformStr, "linuxclang") == 0)
            {
                targetPlatform = TargetPlatform_LinuxClang;
            }
            else if (strcmp(platformStr, "linuxzig") == 0)
            {
                targetPlatform = TargetPlatform_LinuxZig;
            }
            else if (strcmp(platformStr, "linuxsteam") == 0)
            {
                targetPlatform = TargetPlatform_LinuxSteam;
            }
            else
            {
                printf("Unknown platform: %s\n", platformStr);

                return 1;
            }
        }
        else if (strncmp(a_argv[i], BuildConfigurationString, BuildConfigurationStringLen) == 0)
        {
            const char* buildConfigurationStr = a_argv[i] + BuildConfigurationStringLen;
            while (*buildConfigurationStr != '=' && *buildConfigurationStr != '\0')
            {
                ++buildConfigurationStr;
            }

            if (*buildConfigurationStr == '\0')
            {
                printf("Invalid build configuration argument: %s\n", a_argv[i]);

                return 1;
            }

            ++buildConfigurationStr;

            if (strcmp(buildConfigurationStr, "debug") == 0)
            {
                buildConfiguration = BuildConfiguration_Debug;
            }
            else if (strcmp(buildConfigurationStr, "releasewithdebug") == 0)
            {
                buildConfiguration = BuildConfiguration_ReleaseWithDebug;
            }
            else if (strcmp(buildConfigurationStr, "release") == 0)
            {
                buildConfiguration = BuildConfiguration_Release;
            }
            else 
            {
                printf("Unknown build configuration: %s\n", buildConfigurationStr);

                return 1;
            }
        }
        else if (strncmp(a_argv[i], RebuildString, RebuildStringLen) == 0)
        {
            rebuild = CBTRUE;
        }
        else if (strncmp(a_argv[i], EnableTraceString, EnableTraceStringLen) == 0)
        {
            enableTrace = CBTRUE;
        }
        else if (strncmp(a_argv[i], EnableProfilerString, EnableProfilerStringLen) == 0)
        {
            enableProfiler = CBTRUE;
        }
        else if (strncmp(a_argv[i], RemoteString, RemoteStringLen) == 0)
        {
            remoteMode = CBTRUE;
        }
        else if (strncmp(a_argv[i], ModString, ModStringLen) == 0)
        {
            enableMod = CBFALSE;
        }
        else if (strncmp(a_argv[i], JobString, JobStringLen) == 0)
        {
            const char* jobCountStr = a_argv[i] + JobStringLen;
            while (*jobCountStr != '=' && *jobCountStr != 0)
            {
                ++jobCountStr;
            }

            if (*jobCountStr == 0)
            {
                printf("Invalid job count argument: %s\n", a_argv[i]);

                return 1;
            }

            ++jobCountStr;

            jobThreads = (CBUINT32)atoi(jobCountStr);
        }
        else if (strncmp(a_argv[i], NoPrintString, NoPrintStringLen) == 0)
        {
            noPrint = CBTRUE;
        }
        else if (strncmp(a_argv[i], HelpString, HelpStringLen) == 0)
        {
            PrintEngineHelp();

            return 0;
        }
        else if (strcmp(a_argv[i], "-D") == 0)
        {
            buildConfiguration = BuildConfiguration_Debug;
        }
        else if (strcmp(a_argv[i], "-R") == 0)
        {
            buildConfiguration = BuildConfiguration_Release;
        }
        else 
        {
            printf("Unknown argument: %s\n", a_argv[i]);
            printf("\n");

            PrintEngineHelp();

            return 1;
        }
    }

    if (!noPrint)
    {
        printf("IcarianEngine Build\n");
        printf("\n");
    }

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        printf("Target Platform: Windows\n");

        compiler = CUBE_CProjectCompiler_MinGW;

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxSteam:
    {
        printf("Target Platform: Linux\n");

        compiler = CUBE_CProjectCompiler_GCC;

        break;
    }
    case TargetPlatform_LinuxClang:
    {
        printf("Target Platform: Linux Clang\n");

        compiler = CUBE_CProjectCompiler_Clang;

        break;
    }
    case TargetPlatform_LinuxZig:
    {
        printf("Target Platform: Linux Zig\n");

        compiler = CUBE_CProjectCompiler_Zig;

        break;
    }
    }

    CUBE_CommandLine commandLine = { 0 };

    CUBE_String_AppendC(&commandLine.Path, "IcarianNative/lib/glslang");
    CUBE_String_AppendC(&commandLine.Command, "python3");

    CUBE_CommandLine_AppendArgumentC(&commandLine, "update_glslang_sources.py");

    int retCode = CUBE_CommandLine_Execute(&commandLine, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CommandLine_Destroy(&commandLine);

    if (retCode != 0)
    {
        printf("Failed to sync SPIRV-Tools\n");

        return 1;
    }

    dependencyProjects = BuildDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        ret = CUBE_CProject_MultiCompile(&dependencyProjects[i].Project, compiler, dependencyProjects[i].WorkingDirectory, CBNULL, jobThreads, &lines, &lineCount, rebuild);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return 1;
        }

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    icarianCoreProject = BuildIcarianCoreProject("./IcarianCore", CBTRUE, targetPlatform, buildConfiguration);

    ret = CUBE_CProject_MultiCompile(&icarianCoreProject, compiler, "./IcarianCore", CBNULL, jobThreads, &lines, &lineCount, rebuild);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&icarianCoreProject);

    if (!ret)
    {
        printf("Failed to compile IcarianCore\n");

        return 1;
    }

    printf("Writing imports to Header files...\n");
    if (!WriteIcarianCSImportsToHeader("IcarianCS"))
    {
        printf("Failed to write imports to header files\n");

        return 1;
    }

    icarianCSProject = BuildIcarianCSProject(CBTRUE, CBFALSE, CBFALSE, CBFALSE);

    printf("Compiling IcarianCS...\n");
    switch (targetPlatform) 
    {
    case TargetPlatform_LinuxSteam:
    {
        ret = CUBE_CSProject_PreProcessCompile(&icarianCSProject, "IcarianCS", "../deps/Mono/LinuxSteam/bin/csc", compiler, CBNULL, &lines, &lineCount);

        break;
    }
    default:
    {
        ret = CUBE_CSProject_PreProcessCompile(&icarianCSProject, "IcarianCS", "../deps/Mono/Linux/bin/csc", compiler, CBNULL, &lines, &lineCount);

        break;
    }
    }

    FlushLines(&lines, &lineCount);

    CUBE_CSProject_Destroy(&icarianCSProject);

    if (!ret)
    {
        printf("Failed to compile IcarianCS\n");

        return 1;
    }

    printf("Writing shaders to Header files...\n");
    if (!WriteIcarianNativeShadersToHeader("IcarianNative"))
    {
        printf("Failed to write shaders to header files\n");

        return 1;
    }

    dependencyProjects = BuildIcarianNativeDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        ret = CUBE_CProject_MultiCompile(&dependencyProjects[i].Project, compiler, dependencyProjects[i].WorkingDirectory, CBNULL, jobThreads, &lines, &lineCount, rebuild);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return 1;
        }

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    const IcarianNativeProjectFlags nativeFlags =
    {
        .EnableTrace = enableTrace,
        .EnableProfiler = enableProfiler,
        .RemoteMode = remoteMode,
    };
    icarianNativeProject = BuildIcarianNativeProject("./IcarianNative/", targetPlatform, buildConfiguration, nativeFlags);

    ret = CUBE_CProject_MultiCompile(&icarianNativeProject, compiler, "./IcarianNative", CBNULL, jobThreads, &lines, &lineCount, rebuild);

    FlushLines(&lines, &lineCount);

    if (!ret)
    {
        printf("Failed to compile IcarianNative\n");

        return 1;
    }

    CUBE_CProject_Destroy(&icarianNativeProject);

    if (enableMod)
    {
        icarianModManagerProject = BuildIcarianModManagerProject(targetPlatform, buildConfiguration);

        ret = CUBE_CProject_MultiCompile(&icarianModManagerProject, compiler, "IcarianModManager", CBNULL, jobThreads, &lines, &lineCount, rebuild);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile IcarianModManager\n");

            return 1;
        }

        CUBE_CProject_Destroy(&icarianModManagerProject);
    }

    if (!noPrint)
    {
        PrintHeader("Copying Files");
    }

    CUBE_IO_CreateDirectoryC("build");

    CUBE_IO_CopyFileC("IcarianCS/build/IcarianCS.dll", "build/IcarianCS.dll");

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_IO_CopyFileC("IcarianNative/build/IcarianNative.exe", "build/IcarianNative.exe");

        if (enableMod)
        {
            CUBE_IO_CopyFileC("IcarianModManager/build/IcarianModManager.exe", "build/IcarianModManager.exe");
        }

        CUBE_IO_CopyDirectoryC("deps/Mono/Windows/lib/", "build/lib/", CBTRUE);
        CUBE_IO_CopyDirectoryC("deps/Mono/Windows/etc/", "build/etc/", CBTRUE);

        CUBE_IO_CopyFileC("deps/Mono/Windows/bin/mono-2.0-sgen.dll", "build/mono-2.0-sgen.dll");
        CUBE_IO_CopyFileC("deps/Mono/Windows/bin/MonoPosixHelper.dll", "build/MonoPosixHelper.dll");

        break;
    }
    case TargetPlatform_Linux:
    case TargetPlatform_LinuxClang:
    case TargetPlatform_LinuxZig:
    {
        CUBE_IO_CopyFileC("IcarianNative/build/IcarianNative", "build/IcarianNative");

        if (enableMod)
        {
            CUBE_IO_CopyFileC("IcarianModManager/build/IcarianModManager", "build/IcarianModManager");
        }

        CUBE_IO_CHMODC("build/IcarianNative", 0755);
        CUBE_IO_CHMODC("build/IcarianModManager", 0755);

        CUBE_IO_CopyDirectoryC("deps/Mono/Linux/lib/", "build/lib/", CBTRUE);
        CUBE_IO_CopyDirectoryC("deps/Mono/Linux/etc/", "build/etc/", CBTRUE);

        break;
    }
    case TargetPlatform_LinuxSteam:
    {
        CUBE_IO_CopyFileC("IcarianNative/build/IcarianNative", "build/IcarianNative");
        CUBE_IO_CopyFileC("IcarianModManager/build/IcarianModManager", "build/IcarianModManager");

        CUBE_IO_CHMODC("build/IcarianNative", 0755);
        CUBE_IO_CHMODC("build/IcarianModManager", 0755);

        CUBE_IO_CopyDirectoryC("deps/Mono/LinuxSteam/lib/", "build/lib/", CBTRUE);
        CUBE_IO_CopyDirectoryC("deps/Mono/LinuxSteam/etc/", "build/etc/", CBTRUE);

        break;
    }
    }

    if (!noPrint)
    {
        printf("\nDone!\n");
    }

    return 0;
}
