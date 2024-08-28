// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

// Wrapping this header as I was getting compiler warnings with a C++ compiler
// Doing it this way I no longer get warnings they where kinda bugging me seeing them
// Odd considering the header is doing extern C but not gonna question it compilers be weird
extern "C" 
{
// If you are seeing this while debugging in VSCode hope you are ready to see your desktop
// Can also crash when using clangd
// Only been able to open it without crashes in Vim and Kate with clangd disabled
//
// ADDITIONAL NOTE: I have confirmed that this is a know bug with clangd that has been open for a couple years yay...
// However vscode cannot gracefully handle a LSP crash and will occasionsally crash when debugging you have been warned
// #define MA_DEBUG_OUTPUT
#define MA_NO_ENCODING
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_WAV
#define MA_NO_FLAC
#define MA_NO_MP3
#include <miniaudio.h>
}

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
