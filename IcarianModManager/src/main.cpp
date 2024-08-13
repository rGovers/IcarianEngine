// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.