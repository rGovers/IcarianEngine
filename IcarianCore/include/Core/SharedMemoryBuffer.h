// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>

namespace IcarianCore
{
    enum e_SharedMemoryBufferState : uint32_t
    {
        SharedMemoryBufferState_Null,
        SharedMemoryBufferState_Read,
        SharedMemoryBufferState_Write
    };

    struct SharedMemoryBuffer
    {
        // It is imporant that they are all volatile as this is 2 process changing stuff so when optimizations are turned on it would get funky without
        // Compiler will assume we are the only thing that touches this othwise
        volatile e_SharedMemoryBufferState State;
        uint32_t Size;
        // I did a dumb cannot use a pointer due to virtual address space so the data will be after this header
        // NOTE: If you do use pointers they need to be offset pointers not actual pointers
    };
}

// MIT License
//
// Copyright (c) 2025 River Govers
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
