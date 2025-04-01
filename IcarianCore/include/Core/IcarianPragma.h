// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#if defined(__clang__)
#define ICARIAN_PRAGMA(x) _Pragma(#x)
#define ICARIAN_WARNINGPUSH ICARIAN_PRAGMA(clang diagnostic push)
#define ICARIAN_WARNINGPOP ICARIAN_PRAGMA(clang diagnostic pop)
#define ICARIAN_WARNINGSUPPRESS(x) ICARIAN_PRAGMA(clang diagnostic ignored x)
#elif defined(__GNUC__)
#define ICARIAN_PRAGMA(x) _Pragma(#x)
#define ICARIAN_WARNINGPUSH ICARIAN_PRAGMA(GCC diagnostic push)
#define ICARIAN_WARNINGPOP ICARIAN_PRAGMA(GCC diagnostic pop)
#define ICARIAN_WARNINGSUPPRESS(x) ICARIAN_PRAGMA(GCC diagnostic ignored x)
#elif defined(_MSC_VER)
#define ICARIAN_PRAGMA(x) __pragma(#x)
#define ICARIAN_WARNINGPUSH ICARIAN_PRAGMA(warning (push))
#define ICARIAN_WARNINGPOP ICARIAN_PRAGMA(warning (pop))
#define ICARIAN_WARNINGSUPPRESS(x) ICARIAN_PRAGMA(warning (disable : x))
#endif

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