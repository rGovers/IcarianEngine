// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

// The magic 3 macros that make the names format correctly
#define ICARIAN_DEFER_NAMEI(a, b) a##b
#define ICARIAN_DEFER_NAMEM(a, b) ICARIAN_DEFER_NAMEI(a, b)
#define ICARIAN_DEFER_NAME(a) ICARIAN_DEFER_NAMEM(a, __LINE__)

// Improved version of defer
// I have discovered the trinity of auto decltype and using
// Figured out how to get rid of the need to pass a parameter
// A mess that exploits the fact that C++ compilers are aggressive with optimizations and inlining with const values and single use variables
// and to the people that say just use templates, fuck templates
#define IDEFER(code) \
    const auto ICARIAN_DEFER_NAME(_defer) = [&] { code; }; \
    using ICARIAN_DEFER_NAME(_t) = decltype(ICARIAN_DEFER_NAME(_defer)); \
    const struct ICARIAN_DEFER_NAME(_defer_struct) \
    { \
        ICARIAN_DEFER_NAME(_t) m_val; \
        ICARIAN_DEFER_NAME(_defer_struct)(ICARIAN_DEFER_NAME(_t) a_val) : m_val(a_val) { } \
        ~ICARIAN_DEFER_NAME(_defer_struct)() \
        { \
            m_val(); \
        } \
    } ICARIAN_DEFER_NAME(_defer_var)(ICARIAN_DEFER_NAME(_defer))

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