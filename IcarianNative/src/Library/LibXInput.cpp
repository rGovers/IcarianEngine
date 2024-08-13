// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef WIN32

#include "LibXInput.h"

constexpr static const char* Names[] =
{
    "xinput1_4.dll",
    "xinput1_3.dll",
    "xinput9_1_0.dll",
    "xinput1_2.dll",
    "xinput1_1.dll"
};

void* LibXInput::Lib = nullptr;
void* LibXInput::XInputGetState = nullptr;

void LibXInput::Init()
{
    if (Lib == nullptr)
    {
        for (const char* name : Names)
        {
            HMODULE lib = LoadLibraryA(name);
            if (lib != NULL)
            {
                Lib = lib;

                XInputGetState = (void*)GetProcAddress(lib, "XInputGetState");

                break;
            }
        }
    }
}
void LibXInput::Destroy()
{
    if (Lib != nullptr)
    {
        FreeLibrary((HMODULE)Lib);

        Lib = nullptr;
        XInputGetState = nullptr;
    }
}

#endif