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

void PrintHeader(const char* a_str)
{
    printf("\n");
    printf("----------------------------------------\n");
    printf("----------------------------------------\n");
    printf("\n");
    printf("%s\n", a_str);
    printf("\n");
    printf("----------------------------------------\n");
    printf("----------------------------------------\n");
}

static const char PlatformString[] = "--platform";
static const CBUINT32 PlatformStringLen = sizeof(PlatformString) - 1;
static const char BuildConfigurationString[] = "--configuration";
static const CBUINT32 BuildConfigurationStringLen = sizeof(BuildConfigurationString) - 1;
static const char HelpString[] = "--help";
static const CBUINT32 HelpStringLen = sizeof(HelpString) - 1;

void PrintHelp()
{
    printf("Help:\n");

    printf("  --platform=<platform> - Set the target platform. \n");
    printf("    Valid values are: \n");
    printf("      windows - Windows\n");
    printf("      linux - Linux\n");
    printf("\n");

    printf("  --configuration=<configuration> - Set the build configuration. \n");
    printf("    Valid values are: \n");
    printf("      debug - Debug\n");
    printf("      releasewithdebug - Release with debug symbols\n");
    printf("      release - Release\n");
    printf("\n");

    printf("  --help - Print this help message.\n");
    
    printf("\n");
}

void FlushLines(CUBE_String** a_lines, CBUINT32* a_lineCount)
{
    for (CBUINT32 i = 0; i < *a_lineCount; ++i)
    {
        printf("%s\n", (*a_lines)[i].Data);

        CUBE_String_Destroy(&(*a_lines)[i]);
    }

    if (*a_lines != CBNULL)
    {
        free(*a_lines);
        *a_lines = CBNULL;
    }

    *a_lineCount = 0;
}

int main(int a_argc, char** a_argv)
{
    e_TargetPlatform targetPlatform;
    e_BuildConfiguration buildConfiguration;

    CUBE_CProject flareBaseProject;
    CUBE_CSProject icarianCSProject;
    CUBE_CProject icarianNativeProject;

    CBUINT32 dependencyProjectCount;
    DependencyProject* dependencyProjects;

    CBUINT32 lineCount;
    CUBE_String* lines;

    CBBOOL ret;

#ifdef _WIN32
    targetPlatform = TargetPlatform_Windows;
#else
    targetPlatform = TargetPlatform_Linux;
#endif

    buildConfiguration = BuildConfiguration_Debug;

    lineCount = 0;
    lines = CBNULL;

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
        else if (strncmp(a_argv[i], HelpString, HelpStringLen) == 0)
        {
            PrintHelp();

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

            PrintHelp();

            return 1;
        }
    }

    switch (targetPlatform)
    {
    case TargetPlatform_Windows:
    {
        printf("Target Platform: Windows\n");

        break;
    }
    case TargetPlatform_Linux:
    {
        printf("Target Platform: Linux\n");

        break;
    }
    }

    PrintHeader("Building Dependencies");

    printf("Creating Dependencies projects...\n");

    dependencyProjects = BuildDependencies(&dependencyProjectCount, targetPlatform, buildConfiguration);

    printf("Compiling Dependencies...\n");
    for (CBUINT32 i = 0; i < dependencyProjectCount; ++i)
    {
        printf("Compiling %s...\n", dependencyProjects[i].Project.Name.Data);

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, CUBE_CProjectCompiler_GCC, dependencyProjects[i].WorkingDirectory, CBNULL, &lines, &lineCount);

        FlushLines(&lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            free(dependencyProjects);

            return 1;
        }
    }

    free(dependencyProjects);

    PrintHeader("Building FlareBase");

    printf("Creating FlareBase project...\n");
    flareBaseProject = BuildFlareBaseProject(CBTRUE, buildConfiguration);

    printf("Compiling FlareBase...\n");
    ret = CUBE_CProject_Compile(&flareBaseProject, CUBE_CProjectCompiler_GCC, "FlareBase", CBNULL, &lines, &lineCount);

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
    CUBE_CSProject_Compile(&icarianCSProject, "IcarianCS", CBNULL, &lines, &lineCount);

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
    if (!WriteShadersToHeader("IcarianNative"))
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

        ret = CUBE_CProject_Compile(&dependencyProjects[i].Project, CUBE_CProjectCompiler_GCC, dependencyProjects[i].WorkingDirectory, CBNULL, &lines, &lineCount);

        if (!ret)
        {
            printf("Failed to compile %s\n", dependencyProjects[i].Project.Name.Data);

            free(dependencyProjects);

            return 1;
        }
    }

    printf("Done!\n");

    return 0;
}