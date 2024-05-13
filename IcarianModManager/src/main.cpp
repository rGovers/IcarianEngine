#include "Core/WindowsHeaders.h"

#include <cstdio>

#include "Application.h"

#define ICARIANMODMANAGER_VERSION_STRX(x) #x
#define ICARIANMODMANAGER_VERSION_STRI(x) ICARIANMODMANAGER_VERSION_STRX(x)
#define ICARIANMODMANAGER_VERSION_TAGSTR ICARIANMODMANAGER_VERSION_STRI(ICARIANMODMANAGER_VERSION_TAG)
#define ICARIANMODMANAGER_COMMIT_HASHSTR ICARIANMODMANAGER_VERSION_STRI(ICARIANMODMANAGER_COMMIT_HASH)

void PrintVersion()
{
    printf("IcarianModManager %d.%d.%d.%s %s \n", ICARIANMODMANAGER_VERSION_MAJOR, ICARIANMODMANAGER_VERSION_MINOR, ICARIANMODMANAGER_VERSION_PATCH, ICARIANMODMANAGER_COMMIT_HASHSTR, ICARIANMODMANAGER_VERSION_TAGSTR);
}

#ifdef WIN32

int APIENTRY WinMain(HINSTANCE a_hInstance, HINSTANCE a_hPrevInstance, LPSTR a_lpCmdLine, int a_nCmdShow)
{
    PrintVersion();

    Application app = Application();
    app.Run();

    return 0;
}

#else

int main(int a_argc, char** a_argv)
{
    PrintVersion();

    Application app = Application();
    app.Run();

    return 0;
}

#endif