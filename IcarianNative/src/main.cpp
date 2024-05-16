// Windows headers need to be included first and in a specific order otherwise everything breaks
// Cause Windows is a good OS with no flaws at all
// I really fucking hate the WIN32 api
#include "Core/WindowsHeaders.h"

#include <ctime>
#include <stdio.h>
#include <string.h>

#include "Application.h"
#include "Config.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"

#define STBI_ASSERT(x) ICARIAN_ASSERT_MSG(x, "STBI Assert")

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_NO_GIF
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <stb_vorbis.c>

#define ICARIANNATIVE_VERSION_STRX(x) #x
#define ICARIANNATIVE_VERSION_STRI(x) ICARIANNATIVE_VERSION_STRX(x)
#define ICARIANNATIVE_VERSION_TAGSTR ICARIANNATIVE_VERSION_STRI(ICARIANNATIVE_VERSION_TAG)
#define ICARIANNATIVE_COMMIT_HASHSTR ICARIANNATIVE_VERSION_STRI(ICARIANNATIVE_COMMIT_HASH)

void PrintVersion()
{
    printf("IcarianEngine %d.%d.%d.%s %s \n", ICARIANNATIVE_VERSION_MAJOR, ICARIANNATIVE_VERSION_MINOR, ICARIANNATIVE_VERSION_PATCH, ICARIANNATIVE_COMMIT_HASHSTR, ICARIANNATIVE_VERSION_TAGSTR);
}

#ifdef WIN32

int APIENTRY WinMain(HINSTANCE a_hInstance, HINSTANCE a_hPrevInstance, LPSTR a_lpCmdLine, int a_nCmdShow)
{
    PrintVersion();

    Config* config = new Config("./config.xml");

    LPWSTR lpCmdLine = GetCommandLineW();
    int argc;
    LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);

    char** cargv = new char*[argc];
    IDEFER(
    {
        for (int i = 0; i < argc; ++i)
        {
            delete[] cargv[i];
        }

        delete[] cargv;
    });

    for (int i = 0; i < argc; ++i)
    {
        const int len = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, NULL, 0, NULL, NULL);
        cargv[i] = new char[len];
        WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, cargv[i], len, NULL, NULL);

        const char* arg = cargv[i];
        if (strcmp(arg, "--headless") == 0)
        {
            config->SetHeadless(true);
        }
    }

    srand(time(NULL));

    Application app = Application(config);
    app.Run((int32_t)argc, cargv);

    return 0;
}

#else

int main(int a_argc, char* a_argv[])
{
    PrintVersion();

    Config* config = new Config("./config.xml");

    for (int i = 0; i < a_argc; ++i)
    {
        const char* arg = a_argv[i];
        if (strcmp(arg, "--headless") == 0)
        {
            config->SetHeadless(true);
        }
    }

    srand(time(NULL));

    Application app = Application(config);
    app.Run((int32_t)a_argc, a_argv);

    return 0;
}

#endif