// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/COWString.h"

class IO
{
private:

protected:

public:
    static IcarianCore::COWU8String GetFilename(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator);
    static IcarianCore::COWU8String GetExtension(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator);

    static IcarianCore::COWU8String NormalizePath(const char* a_path, IcarianCore::Allocator* a_allocator);
    static IcarianCore::COWU8String NormalizePath(const IcarianCore::CharU8* a_path, IcarianCore::Allocator* a_allocator);
    static IcarianCore::COWU8String NormalizePath(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator);

    static IcarianCore::COWU8String GetTemporaryDirectory(IcarianCore::Allocator* a_allocator);
    static IcarianCore::COWU8String GetCurrentDirectory(IcarianCore::Allocator* a_allocator);

    static IcarianCore::COWU8String CombinePath(const char* a_lhs, const char* a_rhs, IcarianCore::Allocator* a_allocator);
    static IcarianCore::COWU8String CombinePath(const IcarianCore::CharU8* a_lhs, const IcarianCore::CharU8* a_rhs, IcarianCore::Allocator* a_allocator);
    static IcarianCore::COWU8String CombinePath
    (
        const IcarianCore::COWU8String& a_lhs,
        const IcarianCore::COWU8String& a_rhs,
        IcarianCore::Allocator* a_allocator
    );
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
