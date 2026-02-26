// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

constexpr struct EndianStructure
{
private:
    static constexpr uint32_t EndianU32Val = 0x01020304;
    static constexpr uint32_t EndianMagicValue = (const uint8_t&)EndianU32Val;

protected:

public:
    static constexpr bool IsBig = EndianMagicValue == 0x01;
    static constexpr bool IsLittle = EndianMagicValue == 0x04;

    constexpr EndianStructure()
    {
        static_assert(IsLittle != IsBig);
    }

    template<typename T>
    constexpr T FlipEndian(const T& a_val)
    {
        constexpr uint32_t ValSize = sizeof(T);

        T val;
        uint8_t* valPtr = (uint8_t*)&val;
        const uint8_t* dataPtr = (uint8_t*)&a_val;
        for (uint32_t i = 0; i < ValSize; ++i)
        {
            valPtr[i] = dataPtr[ValSize - i - 1];
        }

        return val;
    }

    template<typename T>
    constexpr T FromBigEndian(const void* a_data)
    {
        const T& val = *(T*)a_data;
        if constexpr (IsBig)
        {
            return val;
        }

        return FlipEndian<T>(val);
    }
    template<typename T>
    constexpr T FromLittleEndian(const void* a_data)
    {
        const T& val = *(T*)a_data;
        if constexpr (IsLittle)
        {
            return val;
        }

        return FlipEndian<T>(val);
    }

    template<typename T>
    constexpr T MatchEndian(const void* a_data, bool a_isBig)
    {
        if (a_isBig)
        {
            return FromBigEndian<T>(a_data);
        }

        return FromLittleEndian<T>(a_data);
    }
} Endian;

template<typename T>
constexpr T AlignTo(T a_offset, uintptr_t a_alignment)
{
    return (T)(((a_offset + a_alignment - T(1)) / a_alignment) * a_alignment);
};

template<typename T>
constexpr bool IsAligned(T a_offset, T a_alignment)
{
    return a_offset == AlignTo(a_offset, a_alignment);
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