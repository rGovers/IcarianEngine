// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "IO.h"

COWU8String IO::GetFilename(const COWU8String& a_path, Allocator* a_allocator)
{
    const uint32_t startIndex = ILAMBDA(
    {
        const uint32_t pathSeperator = a_path.FindLastCharacter('/');
        if (pathSeperator != uint32_t(-1))
        {
            ILRETURN pathSeperator;
        }

        ILRETURN uint32_t(0);
    });

    const uint32_t endIndex = ILAMBDA(
    {
        const uint32_t extension = a_path.FindLastCharacter('.');
        if (extension != uint32_t(-1))
        {
            ILRETURN extension;
        }

        ILRETURN a_path.Length();
    });

    return a_path.Substring(startIndex, endIndex, a_allocator);
}

COWU8String IO::GetExtension(const COWU8String& a_path, Allocator* a_allocator)
{
    const uint32_t startIndex = a_path.FindLastCharacter('.');
    if (startIndex == uint32_t(-1))
    {
        return COWU8String(a_allocator);
    }

    const uint32_t endIndex = a_path.Length();
    return a_path.Substring(startIndex, endIndex, a_allocator);
}

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