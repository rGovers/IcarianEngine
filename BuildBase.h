#ifndef INCLUDED_HEADER_BUILDBASE
#define INCLUDED_HEADER_BUILDBASE

#include <stdio.h>
#include <string.h>

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum 
{
    TargetPlatform_Windows,
    TargetPlatform_Linux,
} e_TargetPlatform;

typedef enum
{
    BuildConfiguration_Debug,
    BuildConfiguration_ReleaseWithDebug,
    BuildConfiguration_Release,
} e_BuildConfiguration;

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
        else if (strcmp(extension.Data, ".pix") == 0 || strcmp(extension.Data, ".fpix") == 0)
        {
            fprintf(outputFile, "Pixel");
        }
        
        fprintf(outputFile, "Shader[] =\n");

        CUBE_String filePathStr = CUBE_Path_ToString(&shaderPath);

        FILE* shaderFile = fopen(filePathStr.Data, "r");
        if (shaderFile == NULL)
        {
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

#ifdef __cplusplus
}
#endif

#endif