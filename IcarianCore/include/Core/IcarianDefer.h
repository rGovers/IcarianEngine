// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "Core/IcarianPragma.h"

#define ICARIAN_DEFER_NAMEI(a, b, c) a##_cnt##b##ln##c
#define ICARIAN_DEFER_NAMEM(a, b, c) ICARIAN_DEFER_NAMEI(a, b, c)
#define ICARIAN_DEFER_NAME(x, a) ICARIAN_DEFER_NAMEM(a, x, __LINE__)

#if defined(__GNUC__) && !defined(__clang__)
#define ICARIAN_DEFEROPTIMIZE_ATTRIBUTE __attribute__((optimize("-O3")))
#else
#define ICARIAN_DEFEROPTIMIZE_ATTRIBUTE
#endif

#define ICARIAN_INTERNAL_DEFER(x, code) \
    const auto ICARIAN_DEFER_NAME(x, _defer) = [&] ICARIAN_DEFEROPTIMIZE_ATTRIBUTE { code; }; \
    using ICARIAN_DEFER_NAME(x, _t) = decltype(ICARIAN_DEFER_NAME(x, _defer)); \
    const struct ICARIAN_DEFER_NAME(x, _defer_struct) \
    { \
        ICARIAN_DEFER_NAME(x, _t) m_val; \
        explicit ICARIAN_DEFEROPTIMIZE_ATTRIBUTE ICARIAN_DEFER_NAME(x, _defer_struct)(ICARIAN_DEFER_NAME(x, _t) a_val) : m_val(a_val) { } \
        ICARIAN_DEFEROPTIMIZE_ATTRIBUTE ~ICARIAN_DEFER_NAME(x, _defer_struct)() \
        { \
            m_val(); \
        } \
    } ICARIAN_DEFER_NAME(x, _defer_var)(ICARIAN_DEFER_NAME(x, _defer))

#define IDEFER(code) ICARIAN_INTERNAL_DEFER(__COUNTER__, code)

// MIT License
// 
// Copyright (c) 2026 River Govers
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