// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

class Random
{
private:
    constexpr static uint32_t BufferSize = 2048;

    uint8_t  m_buffer[BufferSize];
    uint32_t m_index;

    void FillBuffer();
    
    Random();

protected:

public:
    ~Random();

    static void Init();
    static void Destroy();

    static uint8_t* GetBytes(uint32_t a_size);

    static uint32_t Range(uint32_t a_min, uint32_t a_max);
    static float Range(float a_min, float a_max);
    static int32_t Range(int32_t a_min, int32_t a_max);
};

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