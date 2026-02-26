// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "DataTypes/COWString.h"

COWU8String COWU8FromASCII(const char* a_str, Allocator* a_allocator)
{
    if (a_str == nullptr)
    {
        return COWU8String(a_allocator);
    }

    const char* slider = a_str;
    while (*slider != 0)
    {
        ++slider;
    }

    const uintptr_t len = slider - a_str;
    // Double the length as it may be full of multibyte characters above char code 128
    CharU8* buffer = a_allocator->ZTAllocate<CharU8>(len * 2 + 1);
    IDEFER(a_allocator->Free(buffer));

    const CharU8* str = (CharU8*)a_str;

    // The lower range of ASCII is compatible with UTF-8 and infact if only using the lower range this function is irrelevant
    // It is when using the upper range that the issues occur hence this function
    uint32_t index = 0;
    for (uint32_t i = 0; i < len; ++i)
    {
        const CharU8 chr = str[i];
        if (chr <= 0x007F)
        {
            buffer[index++] = chr;
        }
        else
        {
            buffer[index++] = (chr >> 6) | 0xC0;
            buffer[index++] = (chr & 0x3F) | 0x80;
        }
    }

    return COWU8String(buffer, index, a_allocator);
}

COWU8String COWU8FromUnicode(const CharU16* a_str, Allocator* a_allocator)
{
    if (a_str == nullptr)
    {
        return COWU8String(a_allocator);
    }

    const CharU16* slider = a_str;
    while (*slider != 0)
    {
        ++slider;
    }

    return COWU8FromUnicode(a_str, (uint32_t)(slider - a_str), a_allocator);
}
COWU8String COWU8FromUnicode(const CharU16* a_str, uint32_t a_length, Allocator* a_allocator)
{
    // Have to assume the worst case that everything is 4 bytes
    CharU8* buffer = a_allocator->ZTAllocate<CharU8>((uint64_t)a_length * 4 + 1);

    // TOOD: Handle endian and BOM
    uint32_t i = 0;
    uint32_t index = 0;
    while (i < a_length)
    {
        // UTF16 to Codepoint
        uint32_t codepoint = 0;
        if (a_str[i] <= 0xD7FF)
        {
            codepoint = a_str[i++];
        }
        else if (a_str[i] <= 0xDBFF)
        {
            if (i + 1 >= a_length)
            {
                break;
            }

            constexpr uint16_t HiSurrogateStart = 0xD800;
            constexpr uint16_t LoSurrogateStart = 0xDC00;

            const CharU16 cHigh = a_str[i++];
            const CharU16 cLow = a_str[i++];

            const uint32_t highSur = ((uint32_t)cHigh - HiSurrogateStart) * 0x400;
            const uint16_t lowSur = ((uint16_t)cLow - LoSurrogateStart);

            codepoint = (lowSur | highSur) + 0x10000;
        }

        // Probably need to do something better but should work.... hopefully....
        if (codepoint == 0)
        {
            break;
        }

        // Codepoint to UTF8
        if (codepoint <= 0x007F)
        {
            buffer[index++] = (CharU8)codepoint;
        }
        else if (codepoint <= 0x07FF)
        {
            buffer[index++] = ((codepoint >> 6) & 0x1F) | 0xC0;
            buffer[index++] = (codepoint & 0x3F) | 0x80;
        }
        else if (codepoint <= 0xFFFF)
        {
            buffer[index++] = ((codepoint >> 12) & 0x0F) | 0xE0;
            buffer[index++] = ((codepoint >> 6) & 0x3F) | 0x80;
            buffer[index++] = (codepoint & 0x3F) | 0x80;
        }
        else if (codepoint <= 0x10FFFF)
        {
            buffer[index++] = ((codepoint >> 18) & 0x07) | 0xF0;
            buffer[index++] = ((codepoint >> 12) & 0x3F) | 0x80;
            buffer[index++] = ((codepoint >> 6) & 0x3F) | 0x80;
            buffer[index++] = (codepoint & 0x3F) | 0x80;
        }
    }

    return COWU8String(buffer, index, a_allocator);
}
COWU8String COWU8FromUnicode(const COWU16String& a_str, Allocator* a_allocator)
{
    const CharU16* data = a_str.StrPtr();
    const uintptr_t len = a_str.Length();

    return COWU8FromUnicode(data, (uint32_t)len, a_allocator);
}
COWU8String COWU8FromUnicode(const CharU32* a_str, Allocator* a_allocator)
{
    const CharU32* slider = a_str;
    while (*slider != 0)
    {
        ++slider;
    }

    return COWU8FromUnicode(a_str, (uint32_t)(slider - a_str), a_allocator);
}
COWU8String COWU8FromUnicode(const CharU32* a_str, uint32_t a_length, Allocator* a_allocator)
{
    // Have to assume the worst case that everything is 4 bytes
    CharU8* buffer = a_allocator->ZTAllocate<CharU8>((uint64_t)a_length * 4 + 1);

    // TOOD: Handle endian and BOM
    uint32_t index = 0;
    for (uint32_t i = 0; i < a_length; ++i)
    {
        // UTF16 to Codepoint
        const uint32_t codepoint = (uint32_t)a_str[i];

        // Probably need to do something better but should work.... hopefully....
        if (codepoint == 0)
        {
            break;
        }

        // Codepoint to UTF8
        if (codepoint <= 0x007F)
        {
            buffer[index++] = (CharU8)codepoint;
        }
        else if (codepoint <= 0x07FF)
        {
            buffer[index++] = ((codepoint >> 6) & 0x1F) | 0xC0;
            buffer[index++] = (codepoint & 0x3F) | 0x80;
        }
        else if (codepoint <= 0xFFFF)
        {
            buffer[index++] = ((codepoint >> 12) & 0x0F) | 0xE0;
            buffer[index++] = ((codepoint >> 6) & 0x3F) | 0x80;
            buffer[index++] = (codepoint & 0x3F) | 0x80;
        }
        else if (codepoint <= 0x10FFFF)
        {
            buffer[index++] = ((codepoint >> 18) & 0x07) | 0xF0;
            buffer[index++] = ((codepoint >> 12) & 0x3F) | 0x80;
            buffer[index++] = ((codepoint >> 6) & 0x3F) | 0x80;
            buffer[index++] = (codepoint & 0x3F) | 0x80;
        }
    }

    return COWU8String(buffer, index, a_allocator);
}
COWU8String COWU8FromUnicode(const COWU32String& a_str, Allocator* a_allocator)
{
    const CharU32* data = a_str.StrPtr();
    const uintptr_t len = a_str.Length();

    return COWU8FromUnicode(data, (uint32_t)len, a_allocator);
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