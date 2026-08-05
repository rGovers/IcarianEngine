// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "IO.h"

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#else
#include <unistd.h>
#endif

#include "IcarianError.h"

IcarianCore::COWU8String IO::GetFilename(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator)
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

IcarianCore::COWU8String IO::GetExtension(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator)
{
    const uint32_t startIndex = a_path.FindLastCharacter('.');
    if (startIndex == uint32_t(-1))
    {
        return IcarianCore::COWU8String(a_allocator);
    }

    const uint32_t endIndex = a_path.Length();
    return a_path.Substring(startIndex, endIndex, a_allocator);
}


IcarianCore::COWU8String IO::NormalizePath(const char* a_path, IcarianCore::Allocator* a_allocator)
{
    IcarianCore::COWU8String str = IcarianCore::COWU8String(a_allocator);

    const char* slider = a_path;
    while (true)
    {
        IDEFER(++slider);

        const char chr = *slider;
        if (chr == 0)
        {
            break;
        }

        switch (chr)
        {
        case '\\':
        {
            // Not sure if the string will be formated with 1 or 2 which is part of the problem
            if (*(slider + 1) == '\\')
            {
                ++slider;
            }

            str.Append('/');

            break;
        }
        default:
        {
            ICARIAN_ASSERT(chr >= 0);

            str.Append(chr);

            break;
        }
        }
    }

    return str;
}
IcarianCore::COWU8String IO::NormalizePath(const IcarianCore::CharU8* a_path, IcarianCore::Allocator* a_allocator)
{
    IcarianCore::COWU8String str = IcarianCore::COWU8String(a_allocator);

    const IcarianCore::CharU8* slider = a_path;
    while (true)
    {
        IDEFER(++slider);

        const IcarianCore::CharU8 chr = *slider;
        if (chr == 0)
        {
            break;
        }

        switch (chr)
        {
        case '\\':
        {
            // Not sure if the string will be formated with 1 or 2 which is part of the problem
            if (*(slider + 1) == '\\')
            {
                ++slider;
            }

            str.Append('/');

            break;
        }
        default:
        {
            str.Append(chr);

            break;
        }
        }
    }

    return str;
}
IcarianCore::COWU8String IO::NormalizePath(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator)
{
    const IcarianCore::CharU8* path = a_path.Data();

    return NormalizePath(path, a_allocator);
}

IcarianCore::COWU8String IO::GetTemporaryDirectory(IcarianCore::Allocator* a_allocator)
{
#ifdef WIN32
    wchar_t buffer[MAX_PATH + 1];
    memset(buffer, 0, sizeof(buffer));

    GetTempPathW(sizeof(buffer) - 1, buffer);

    // Windows is annoying as they started using Unicode before it was sorted so they have a bastardized version of UTF-16
    // We need to get Windows to convert it to actual Unicode
    // Yes Windows says it is UTF-16 but they have some historic quirks so do not use directly
    const int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);

    IcarianCore::CharU8* mBuff = a_allocator->ZTAllocate<IcarianCore::CharU8>(len + 1);
    IDEFER(a_allocator->Free(mBuff));

    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, (LPSTR)mBuff, len, NULL, NULL);

    return NormalizePath(mBuff, a_allocator);
#else
    constexpr const char* TempTable[] =
    {
        "TMPDIR",
        "TMP",
        "TEMPDIR",
        "TEMP"
    };

    for (const char* temp : TempTable)
    {
        const char* dir = getenv(temp);
        if (dir != NULL)
        {
            return IcarianCore::COWU8String(dir, a_allocator);
        }
    }

    // Annoying but I am pretty sure UNIX just goes give up and return /tmp but not sure if it is even required to exist
    // If this gives us issues may have to rethink things and create our own temp dir
    return IcarianCore::COWU8String("/tmp", a_allocator);
#endif

    IERROR("Unreachable path hit");

    return IcarianCore::COWU8String(a_allocator);
}
IcarianCore::COWU8String IO::GetCurrentDirectory(IcarianCore::Allocator* a_allocator)
{
#ifdef WIN32
    wchar_t buffer[MAX_PATH + 1];
    memset(buffer, 0, sizeof(buffer));

    GetCurrentDirectoryW(sizeof(buffer) - 1, buffer);

    // Windows is annoying as they started using Unicode before it was sorted so they have a bastardized version of UTF-16
    // We need to get Windows to convert it to actual Unicode
    // Yes Windows says it is UTF-16 but they have some historic quirks so do not use directly
    const int len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);

    IcarianCore::CharU8* mBuff = a_allocator->ZTAllocate<IcarianCore::CharU8>(len + 1);
    IDEFER(a_allocator->Free(mBuff));

    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, (LPSTR)mBuff, len, NULL, NULL);

    return NormalizePath(mBuff, a_allocator);
#else
    char* buffer = getcwd(NULL, 0);
    IDEFER(free(buffer));

    return IcarianCore::COWU8String(buffer, a_allocator);
#endif

    IERROR("Unreachable path hit");

    return IcarianCore::COWU8String(a_allocator);
}

IcarianCore::COWU8String IO::CombinePath(const char* a_lhs, const char* a_rhs, IcarianCore::Allocator* a_allocator)
{
    const uint32_t lhsLen = ILAMBDA(
    {
        const char* slider = a_lhs;
        while (*slider != 0)
        {
            ++slider;
        }

        ILRETURN (uint32_t)(slider - a_lhs);
    });

    IcarianCore::COWU8String str = IcarianCore::COWU8String(a_lhs, lhsLen, a_allocator);

    if (lhsLen > 0 && a_lhs[lhsLen - 1] != '/')
    {
        str.Append('/');
    }

    str.Append(a_rhs);

    return str;
}
IcarianCore::COWU8String IO::CombinePath(const IcarianCore::CharU8* a_lhs, const IcarianCore::CharU8* a_rhs, IcarianCore::Allocator* a_allocator)
{
    const uint32_t lhsLen = ILAMBDA(
    {
        const IcarianCore::CharU8* slider = a_lhs;
        while (*slider != 0)
        {
            ++slider;
        }

        ILRETURN (uint32_t)(slider - a_lhs);
    });

    IcarianCore::COWU8String str = IcarianCore::COWU8String(a_lhs, lhsLen, a_allocator);

    if (lhsLen > 0 && a_lhs[lhsLen - 1] != '/')
    {
        str.Append('/');
    }

    str.Append(a_rhs);

    return str;
}
IcarianCore::COWU8String IO::CombinePath(const IcarianCore::COWU8String& a_lhs, const IcarianCore::COWU8String& a_rhs, IcarianCore::Allocator* a_allocator)
{
    IcarianCore::COWU8String str = IcarianCore::COWU8String(a_lhs, a_allocator);

    const uint32_t len = a_lhs.Length();
    if (len > 0 && a_lhs[len - 1] != '/')
    {
        str.Append("/");
    }

    str.Append(a_rhs);

    return str;
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
