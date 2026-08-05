// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <type_traits>

#include "Core/DataTypes/COWString.h"

namespace IcarianCore
{
    template<typename T, typename = void>
    struct DefaultLesserFuntionImpl { };

    template<typename T>
    struct DefaultLesserFunction
    {
        static bool Less(const T& a_lhs, const T& a_rhs)
        {
            return DefaultLesserFuntionImpl<std::decay_t<T>>::Less(a_lhs, a_rhs);
        }
    };

    template<>
    struct DefaultLesserFuntionImpl<uint32_t>
    {
        static bool Less(uint32_t a_lhs, uint32_t a_rhs)
        {
            return a_lhs < a_rhs;
        }
    };
    template<>
    struct DefaultLesserFuntionImpl<COWU8String>
    {
        static bool Less(const COWU8String& a_lhs, const COWU8String& a_rhs)
        {
            const char* lhs = a_lhs.CStr();
            const char* rhs = a_rhs.CStr();

            const char* lhsSlider = lhs;
            const char* rhsSlider = rhs;

            while (true)
            {
                if (*lhsSlider < *rhsSlider)
                {
                    return true;
                }

                if (*lhsSlider > *rhsSlider)
                {
                    return false;
                }

                if (*lhsSlider == 0)
                {
                    break;
                }

                if (*rhsSlider == 0)
                {
                    break;
                }

                ++lhsSlider;
                ++rhsSlider;
            }

            return false;
        }
    };
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
