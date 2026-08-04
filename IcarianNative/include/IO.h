// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "DataTypes/COWString.h"

class IO
{
private:

protected:

public:
    static COWU8String GetFilename(const COWU8String& a_path, Allocator* a_allocator);
    static COWU8String GetExtension(const COWU8String& a_path, Allocator* a_allocator);

    static COWU8String NormalizePath(const char* a_path, Allocator* a_allocator);
    static COWU8String NormalizePath(const CharU8* a_path, Allocator* a_allocator);
    static COWU8String NormalizePath(const COWU8String& a_path, Allocator* a_allocator);

    static COWU8String GetTemporaryDirectory(Allocator* a_allocator);
    static COWU8String GetCurrentDirectory(Allocator* a_allocator);

    static COWU8String CombinePath(const char* a_lhs, const char* a_rhs, Allocator* a_allocator);
    static COWU8String CombinePath(const CharU8* a_lhs, const CharU8* a_rhs, Allocator* a_allocator);
    static COWU8String CombinePath(const COWU8String& a_lhs, const COWU8String& a_rhs, Allocator* a_allocator);
};

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
