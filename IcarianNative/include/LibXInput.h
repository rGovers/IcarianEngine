#pragma once

#ifdef WIN32

#include "Core/WindowsHeaders.h"

#include <xinput.h>

typedef DWORD (WINAPI* PFN_XInputGetState)(DWORD, XINPUT_STATE*);

class LibXInput
{
private:
    static void* Lib;

    LibXInput();

protected:

public:
    ~LibXInput();

    static void Init();
    static void Destroy();

    static void* XInputGetState;
};

#endif