#define CUBE_IMPLEMENTATION
#include "CUBE/CUBE.h"

#include "BuildFlareBase.h"

int main(int a_argc, char** a_argv)
{
    CUBE_CProject flareBaseProject = BuildFlareBaseProject(CBTRUE);

    CUBE_CProject_Compile(&flareBaseProject, CUBE_CProjectCompiler_GCC, "FlareBase", CBNULL);

    CUBE_CProject_Destroy(&flareBaseProject);

    return 0;
}