// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;

namespace IcarianEngine
{
    public class EditorRangeAttribute : Attribute
    {
        double m_min;
        double m_max;

        /// <summary>
        /// The minimum value for the field
        /// </summary>
        public double Min
        {
            get
            {
                return m_min;
            }
        }
        /// <summary>
        /// The maximum value for the field
        /// </summary>
        public double Max
        {
            get
            {
                return m_max;
            }
        }

        public EditorRangeAttribute(double a_min, double a_max)
        {
            m_min = a_min;
            m_max = a_max;
        }
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
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