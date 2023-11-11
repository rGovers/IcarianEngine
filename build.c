#define CUBE_IMPLEMENTATION
#define CUBE_PRINT_COMMANDS
#include "CUBE/CUBE.h"

#include <stdio.h>
#include <string.h>

#include "BuildBase.h"

#include "deps/BuildDependencies.h"
#include "FlareBase/BuildFlareBase.h"
#include "IcarianCS/BuildIcarianCS.h"
#include "IcarianNative/BuildIcarianNative.h"

void PrintEngineHelp()
{
    PrintHelp();

    printf("Engine Arguments:\n");

    printf("  --enable-trace - Enables debug logging for the engine \n");
    printf("  --enable-profiler - Enables the internal profiler for the engine \n");
}

static const char EnableTraceString[] = "--enable-trace";
static const CBUINT32 EnableTraceStringLen = sizeof(EnableTraceString) - 1;
static const char EnableProfilerString[] = "--enable-profiler";
static const CBUINT32 EnableProfilerStringLen = sizeof(EnableProfilerString) - 1;

int main(int a_argc, char** a_argv)
{
    e_TargetPlatform targetPlatform;
    e_BuildConfiguration buildConfiguration;

    CUBE_CProject flareBaseProject;
    CUBE_CSProject icarianCSProject;
    CUBE_CProject icarianNativeProject;

    e_CUBE_CProjectCompiler compiler;

    CBUINT32 dependencyProjectCount;
    DependencyProject* dependencyProjects;

    CBUINT32 lineCount;
    CUBE_String* lines;

    CBBOOL ret;

    CBBOOL enableTrace;
    CBBOOL enableProfiler;

#ifdef _WIN32
    targetPlatform = TargetPlatform_Windows;
#else
    targetPlatform = TargetPlatform_Linux;
#endif

    buildConfiguration = BuildConfiguration_Debug;

    lineCount = 0;
    lines = CBNULL;

    enableTrace = CBFALSE;
    enableProfiler = CBFALSE;

    printf("IcarianEngine Build\n");
    printf("\n");

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
        else if (strncmp(a_argv[i], EnableTraceString, EnableTraceStringLen) == 0)
        {
            enableTrace = CBTRUE;
        }
        else if (strncmp(a_argv[i], EnableProfilerString, EnableProfilerStringLen) == 0)
        {
            enableProfiler = CBTRUE;
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

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        printf("Target Platform: Windows\n");

        compiler = CUBE_CProjectCompiler_MinGW;

        break;
    }
    case TargetPlatform_Linux:
    {
        printf("Target Platform: Linux\n");

        compiler = CUBE_CProjectCompiler_GCC;

        break;
    }
    }

    CUBE_CommandLine commandLine = { 0 };

    CUBE_String_AppendC(&commandLine.Path, "IcarianNative/lib/SPIRV-Tools");
    CUBE_String_AppendC(&commandLine.Command, "python3");

    CUBE_CommandLine_AppendArgumentC(&commandLine, "utils/git-sync-deps");

    int retCode = CUBE_CommandLine_Execute(&commandLine, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CommandLine_Destroy(&commandLine);

    if (retCode != 0)
    {
        printf("Failed to sync SPIRV-Tools\n");

        return 1;
    }

    PrintHeader("Building Dependencies");

    printf("Creating Dependencies projects...\n");

    dependencyProjects = BuildDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    printf("Compiling Dependencies...\n");
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, compiler, dependencyProjects[i].WorkingDirectory, CBNULL, &lines, &lineCount);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return 1;
        }

        printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    PrintHeader("Building FlareBase");

    printf("Creating FlareBase project...\n");
    flareBaseProject = BuildFlareBaseProject(CBTRUE, targetPlatform, buildConfiguration);

    printf("Compiling FlareBase...\n");
    ret = CUBE_CProject_Compile(&flareBaseProject, compiler, "FlareBase", CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CProject_Destroy(&flareBaseProject);

    if (!ret)
    {
        printf("Failed to compile FlareBase\n");

        return 1;
    }

    printf("FlareBase Compiled!\n");

    PrintHeader("Building IcarianCS");

    printf("Creating IcarianCS project...\n");
    icarianCSProject = BuildIcarianCSProject(CBTRUE);

    printf("Compiling IcarianCS...\n");
    ret = CUBE_CSProject_PreProcessCompile(&icarianCSProject, "IcarianCS", "../deps/Mono/Linux/bin/csc", compiler, CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    CUBE_CSProject_Destroy(&icarianCSProject);

    if (!ret)
    {
        printf("Failed to compile IcarianCS\n");

        return 1;
    }

    printf("IcarianCS Compiled!\n");

    PrintHeader("Building IcarianNative");

    printf("Writing shaders to Header files...\n");
    if (!WriteIcarianNativeShadersToHeader("IcarianNative"))
    {
        printf("Failed to write shaders to header files\n");

        return 1;
    }

    printf("Creating IcarianNative Dependencies projects...\n");
    dependencyProjects = BuildIcarianNativeDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    printf("Compiling IcarianNative Dependencies...\n");
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, compiler, dependencyProjects[i].WorkingDirectory, CBNULL, &lines, &lineCount);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            return 1;
        }

        printf("Compiled %s\n", dependencyProjects[i].Project.Name.Data);

        CUBE_CProject_Destroy(&dependencyProjects[i].Project);
    }

    free(dependencyProjects);

    printf("Creating IcarianNative project...\n");
    icarianNativeProject = BuildIcarianNativeProject(targetPlatform, buildConfiguration, enableTrace, enableProfiler);

    printf("Compiling IcarianNative...\n");
    ret = CUBE_CProject_Compile(&icarianNativeProject, compiler, "IcarianNative", CBNULL, &lines, &lineCount);

    FlushLines(&lines, &lineCount);

    if (!ret)
    {
        printf("Failed to compile IcarianNative\n");

        return 1;
    }

    printf("IcarianNative Compiled!\n");

    CUBE_CProject_Destroy(&icarianNativeProject);

    PrintHeader("Copying Files");

    CUBE_IO_CreateDirectoryC("build");

    CUBE_IO_CopyFileC("IcarianCS/build/IcarianCS.exe", "build/IcarianCS.dll");

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        CUBE_IO_CopyFileC("IcarianNative/build/IcarianNative.exe", "build/IcarianNative.exe");

        CUBE_IO_CopyDirectoryC("deps/Mono/Windows/lib/", "build/lib/", CBTRUE);
        CUBE_IO_CopyDirectoryC("deps/Mono/Windows/etc/", "build/etc/", CBTRUE);

        CUBE_IO_CopyFileC("deps/Mono/Windows/bin/mono-2.0-sgen.dll", "build/mono-2.0-sgen.dll");
        CUBE_IO_CopyFileC("deps/Mono/Windows/bin/MonoPosixHelper.dll", "build/MonoPosixHelper.dll");

        break;
    }
    case TargetPlatform_Linux:
    {
        CUBE_IO_CopyFileC("IcarianNative/build/IcarianNative", "build/IcarianNative");

        CUBE_IO_CHMODC("build/IcarianNative", 0755);

        CUBE_IO_CopyDirectoryC("deps/Mono/Linux/lib/", "build/lib/", CBTRUE);
        CUBE_IO_CopyDirectoryC("deps/Mono/Linux/etc/", "build/etc/", CBTRUE);

        break;
    }
    }

    printf("Done!\n");

    return 0;
}