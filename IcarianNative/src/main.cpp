#include <ctime>
#include <stdio.h>
#include <string.h>

#include "Application.h"
#include "Config.h"
#include "Flare/IcarianAssert.h"

#define STBI_ASSERT(x) ICARIAN_ASSERT_MSG(x, "STBI Assert")

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <stb_vorbis.c>

int main(int a_argc, char* a_argv[])
{
    Config* config = new Config("./config.xml");

    printf("IcarianEngine %d.%d \n", ICARIANNATIVE_VERSION_MAJOR, ICARIANNATIVE_VERSION_MINOR);

    srand(time(NULL));

    for (int i = 0; i < a_argc; ++i)
    {
        const char* arg = a_argv[i];
        if (strcmp(arg, "--headless") == 0)
        {
            config->SetHeadless(true);
        }
    }

    Application app = Application(config);
    app.Run((int32_t)a_argc, a_argv);

    return 0;
}