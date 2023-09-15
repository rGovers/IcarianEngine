#ifndef INCLUDED_HEADER_BUILDFLAREBASE
#define INCLUDED_HEADER_BUILDFLAREBASE

#include "CUBE/CUBE.h"

#ifdef __cplusplus
extern "C" {
#endif

CUBE_CProject BuildFlareBaseProject(CBBOOL a_enableAssert)
{
    CUBE_CProject project = { 0 };

    project.Name = CUBE_StackString_CreateC("FlareBase");
    project.Target = CUBE_CProjectTarget_StaticLibrary;
    project.Language = CUBE_CProjectLanguage_CPP;
    project.OutputPath = CUBE_Path_CreateC("./build/");

    if (a_enableAssert)
    {
        CUBE_CProject_AppendDefine(&project, "FLARE_ENABLE_ASSERT");
    }

    CUBE_CProject_AppendDefine(&project, "GLM_FORCE_QUAT_DATA_XYZW");
    CUBE_CProject_AppendDefine(&project, "GLM_FORCE_DEPTH_ZERO_TO_ONE");
    CUBE_CProject_AppendDefine(&project, "GLM_FORCE_RADIANS");

    CUBE_CProject_AppendIncludePath(&project, "include");
    CUBE_CProject_AppendIncludePath(&project, "../deps/flare-glm");
    CUBE_CProject_AppendIncludePath(&project, "../deps/flare-tinyxml2");

    CUBE_CProject_AppendSource(&project, "src/ColladaLoader.cpp");
    CUBE_CProject_AppendSource(&project, "src/InputBindings.cpp");
    CUBE_CProject_AppendSource(&project, "src/IPCPipe.cpp");
    CUBE_CProject_AppendSource(&project, "src/OBJLoader.cpp");
    CUBE_CProject_AppendSource(&project, "../deps/flare-tinyxml2/tinyxml2.cpp");
    
    CUBE_CProject_AppendReference(&project, "stdc++");

    return project;
}

#ifdef __cplusplus
}
#endif

#endif