#ifndef INCLUDED_HEADER_BUILDBASE
#define INCLUDED_HEADER_BUILDBASE

#include <stdio.h>
#include <string.h>

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
    CUBE_CProject Project;
    const char* WorkingDirectory;
} DependencyProject;

typedef enum 
{
    TargetPlatform_Windows,
    TargetPlatform_Linux,
    TargetPlatform_LinuxClang,
    TargetPlatform_LinuxZig
} e_TargetPlatform;

typedef enum
{
    BuildConfiguration_Debug,
    BuildConfiguration_ReleaseWithDebug,
    BuildConfiguration_Release,
} e_BuildConfiguration;

CBBOOL TemplatesToHeader(const CUBE_Path* a_templatePath, CBUINT32 a_templateCount, const char* a_outputFile)
{
    FILE* outputFile = fopen(a_outputFile, "w");
    if (outputFile == NULL)
    {
        return CBFALSE;
    }

    fprintf(outputFile, "#pragma once\n\n");

    for (CBUINT32 i = 0; i < a_templateCount; ++i)
    {
        const CUBE_Path templatePath = a_templatePath[i];

        CUBE_String templateName = CUBE_Path_Filename(&templatePath);

        fprintf(outputFile, "constexpr static char %sTemplate[] =\n", templateName.Data);
        
        CUBE_String filePathStr = CUBE_Path_ToString(&templatePath);

        FILE* templateFile = fopen(filePathStr.Data, "r");
        if (templateFile == NULL)
        {
            fclose(outputFile);

            return CBFALSE;
        }

        fprintf(outputFile, "\"");

        char buffer[1024];
        while (fgets(buffer, 1024, templateFile) != NULL)
        {
            const char* s = buffer;
            const char* e = buffer;

            char line[1024];

            while (*s != '\0')
            {
                if (*s == '\n')
                {
                    memcpy(line, e, s - e);
                    line[s - e] = '\0';

                    fprintf(outputFile, "%s\\n\\\n", line);

                    e = s + 1;
                }
                else if (*s == '\"')
                {
                    memcpy(line, e, s - e);
                    line[s - e] = '\0';

                    fprintf(outputFile, "%s\\\"", line);

                    e = s + 1;
                }

                ++s;
            }

            if (e != s)
            {
                fprintf(outputFile, "%s", e);
            }
        }

        fclose(templateFile);

        fprintf(outputFile, "\";\n\n");

        CUBE_String_Destroy(&filePathStr);
        CUBE_String_Destroy(&templateName);
    }

    fclose(outputFile);

    return CBTRUE;
}
CBBOOL ShadersToHeader(const CUBE_Path* a_shaderPaths, CBUINT32 a_shaderCount, const char* a_outputFile)
{
    FILE* outputFile = fopen(a_outputFile, "w");
    if (outputFile == NULL)
    {
        return CBFALSE;
    }

    fprintf(outputFile, "#pragma once\n\n");

    for (CBUINT32 i = 0; i < a_shaderCount; ++i)
    {
        const CUBE_Path shaderPath = a_shaderPaths[i];

        CUBE_String shaderName = CUBE_Path_Filename(&shaderPath);

        fprintf(outputFile, "constexpr static char %s", shaderName.Data);

        CUBE_String extension = CUBE_Path_Extension(&shaderPath);

        if (strcmp(extension.Data, ".vert") == 0 || strcmp(extension.Data, ".fvert") == 0)
        {
            fprintf(outputFile, "Vertex");
        }
        else if (strcmp(extension.Data, ".pix") == 0 || strcmp(extension.Data, ".frag") == 0 || strcmp(extension.Data, ".fpix") == 0)
        {
            fprintf(outputFile, "Pixel");
        }
        
        fprintf(outputFile, "Shader[] =\n");

        CUBE_String filePathStr = CUBE_Path_ToString(&shaderPath);

        FILE* shaderFile = fopen(filePathStr.Data, "r");
        if (shaderFile == NULL)
        {
            fclose(outputFile);

            return CBFALSE;
        }

        fprintf(outputFile, "\"");

        char buffer[1024];
        while (fgets(buffer, 1024, shaderFile) != NULL)
        {
            const char* s = buffer;
            const char* e = buffer;

            char line[1024];

            while (*s != '\0')
            {
                if (*s == '\n')
                {
                    memcpy(line, e, s - e);
                    line[s - e] = '\0';

                    // Me brain no worky
                    // String shananigans to get the line to print correctly
                    fprintf(outputFile, "%s\\n\\\n", line);

                    e = s + 1;
                }

                ++s;
            }

            if (e != s)
            {
                fprintf(outputFile, "%s", e);
            }
        }

        fclose(shaderFile);

        fprintf(outputFile, "\";\n\n");

        CUBE_String_Destroy(&filePathStr);
        CUBE_String_Destroy(&extension);
        CUBE_String_Destroy(&shaderName);
    }

    fclose(outputFile);

    return CBTRUE;       
}

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
static const char CompileCommandsString[] = "--compile-commands";
static const CBUINT32 CompileCommandsStringLen = sizeof(CompileCommandsString) - 1;

void PrintHelp()
{
    printf("Help:\n");

    printf("  --compile-commands=<workingDirectory> - Generate a compile commands file. \n");
    printf("\n");

    printf("  --platform=<platform> - Set the target platform. \n");
    printf("    Valid values are: \n");
    printf("      windows - Windows\n");
    printf("      linux - Linux with GCC\n");
    printf("      linuxclang - Linux with Clang\n");
    printf("      linuxzig - Linux with Zig\n");
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

#ifdef __cplusplus
}
#endif

#endif