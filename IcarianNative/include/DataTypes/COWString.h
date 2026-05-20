// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

class Allocator;
template<typename CharType>
class COWBasicString;

// Grumble Grumble
// Have to define our own char types as for some reason char8_t is missing until C++20/C23
// GCC pulled char8_t back to C++11 so yeah standards thing despite the fact it will compile without warnings
// Clang also has a habbit of copying GCC so yeah....
// Not the end of the world as Unicode is built around 8, 16 and 32 bit integer types just annoying
typedef uint8_t CharU8;
typedef uint16_t CharU16;
typedef uint32_t CharU32;

using COWU8String = COWBasicString<CharU8>;
using COWU16String = COWBasicString<CharU16>;
using COWU32String = COWBasicString<CharU32>;

// TODO: Will need multiple version of this function as the extended ASCII range is a mess
// Probably want to use the actual names when we do implement the other versions
COWU8String COWU8FromASCII(const char* a_str, Allocator* a_allocator);

COWU8String COWU8FromUnicode(const CharU16* a_str, Allocator* a_allocator);
COWU8String COWU8FromUnicode(const CharU16* a_str, uint32_t a_length, Allocator* a_allocator);
COWU8String COWU8FromUnicode(const COWU16String& a_str, Allocator* a_allocator);
COWU8String COWU8FromUnicode(const CharU32* a_str, Allocator* a_allocator);
COWU8String COWU8FromUnicode(const CharU32* a_str, uint32_t a_length, Allocator* a_allocator);
COWU8String COWU8FromUnicode(const COWU32String& a_str, Allocator* a_allocator);

#include "DataTypes/COWBasicString.h"

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