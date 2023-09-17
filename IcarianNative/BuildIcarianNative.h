#ifndef INCLUDED_HEADER_BUILDICARIANNATIVE
#define INCLUDED_HEADER_BUILDICARIANNATIVE

#include "CUBE/CUBE.h"

#include "../BuildBase.h"
#include "lib/BuildIcarianNativeDependencies.h"

#ifdef __cplusplus
extern "C" {
#endif

const static char* BasePaths[] =
{
    "shaders/DirectionalLight.fpix",
    "shaders/PointLight.fpix",
    "shaders/Post.fpix",
    "shaders/Quad.vert",
    "shaders/SpotLight.fpix",
    "shaders/UI.fvert",
    "shaders/UIImage.fpix",
    "shaders/UIText.fpix"
};

const static CBUINT32 BasePathCount = sizeof(BasePaths) / sizeof(char*);

CBBOOL WriteShadersToHeader(const char* a_workingPath)
{
    CUBE_Path workingPath = CUBE_Path_CreateC(a_workingPath);

    CUBE_Path shaderPaths[BasePathCount];

    for (CBUINT32 i = 0; i < BasePathCount; ++i)
    {
        shaderPaths[i] = CUBE_Path_CombineC(&workingPath, BasePaths[i]);
    }

    CUBE_Path outPath = CUBE_Path_CombineC(&workingPath, "include/Shaders.h");
    CUBE_String outPathStr = CUBE_Path_ToString(&outPath);

    const CBBOOL ret = ShadersToHeader(shaderPaths, BasePathCount, outPathStr.Data);

    CUBE_String_Destroy(&outPathStr);
    CUBE_Path_Destroy(&outPath);

    for (CBUINT32 i = 0; i < BasePathCount; ++i)
    {
        CUBE_Path_Destroy(&shaderPaths[i]);
    }

    CUBE_Path_Destroy(&workingPath);

    return ret;
}

DependencyProject* BuildIcarianNativeDependencies(CBUINT32* a_count, e_TargetPlatform a_targetPlatform, e_BuildConfiguration a_configuration)
{
    return BuildIcarianNativeIDependencies(a_count, a_targetPlatform, a_configuration);
}

#ifdef __cplusplus
}
#endif

#endif