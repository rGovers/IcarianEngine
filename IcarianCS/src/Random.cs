// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine
{
    public static class Random
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void FillBuffer(byte[] a_buffer, uint a_offset);

        const uint BufferSize = 512;

        static byte[] s_buffer = new byte[BufferSize];
        static uint s_bufferIndex = BufferSize;

        static void FillBuffer()
        {
            uint offset = BufferSize - s_bufferIndex;
            for (int i = 0; i < offset; ++i)
            {
                s_buffer[i] = s_buffer[i + s_bufferIndex];
            }

            FillBuffer(s_buffer, offset);

            s_bufferIndex = 0;
        }

        /// <summary>
        /// Generates a random int
        /// </summary>
        /// <returns>A random int value</returns>
        public static int Int()
        {
            if (s_bufferIndex + 4 >= BufferSize)
            {
                FillBuffer();
            }

            int value = BitConverter.ToInt32(s_buffer, (int)s_bufferIndex);

            s_bufferIndex += 4;

            return value;
        }
        /// <summary>
        /// Generates a random uint
        /// </summary>
        /// <returns>A random uint value</returns>
        public static uint UInt()
        {
            if (s_bufferIndex + 4 >= BufferSize)
            {
                FillBuffer();
            }

            uint value = BitConverter.ToUInt32(s_buffer, (int)s_bufferIndex);

            s_bufferIndex += 4;

            return value;
        }

        /// <summary>
        /// Generates a random uint in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static uint Range(uint a_min, uint a_max)
        {
            return a_min + (UInt() % (a_max - a_min + 1));
        }
        /// <summary>
        /// Generates a random float in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static float Range(float a_min, float a_max)
        {
            double mul = UInt() / (double)uint.MaxValue;
            float range = a_max - a_min;

            return (float)(a_min + (mul * range));
        }
        /// <summary>
        /// Generates a random int in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static int Range(int a_min, int a_max)
        {
            return a_min + (Int() % (a_max - a_min + 1));
        }

        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.Vector2" /> in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static Vector2 Range(Vector2 a_min, Vector2 a_max)
        {
            return new Vector2(Range(a_min.X, a_max.X), Range(a_min.Y, a_max.Y));
        }
        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.Vector3" /> in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static Vector3 Range(Vector3 a_min, Vector3 a_max)
        {
            return new Vector3(Range(a_min.X, a_max.X), Range(a_min.Y, a_max.Y), Range(a_min.Z, a_max.Z));
        }
        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.Vector4" /> in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static Vector4 Range(Vector4 a_min, Vector4 a_max)
        {
            return new Vector4(Range(a_min.X, a_max.X), Range(a_min.Y, a_max.Y), Range(a_min.Z, a_max.Z), Range(a_min.W, a_max.W));
        }

        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.Quaternion" />
        /// </summary>
        /// <returns>A normalized random <see cref="IcarianEngine.Maths.Quaternion" /></returns>
        public static Quaternion Rotation()
        {
            return Quaternion.Normalized(new Quaternion(Range(-1.0f, 1.0f), Range(-1.0f, 1.0f), Range(-1.0f, 1.0f), Range(-1.0f, 1.0f)));
        }

        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.IVector2" /> in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static IVector2 Range(IVector2 a_min, IVector2 a_max)
        {
            return new IVector2(Range(a_min.X, a_max.X), Range(a_min.Y, a_max.Y));
        }
        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.IVector3" /> in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static IVector3 Range(IVector3 a_min, IVector3 a_max)
        {
            return new IVector3(Range(a_min.X, a_max.X), Range(a_min.Y, a_max.Y), Range(a_min.Z, a_max.Z));
        }
        /// <summary>
        /// Generates a random <see cref="IcarianEngine.Maths.IVector4" /> in range
        /// </summary>
        /// <param name="a_min">The minimum value inclusive</param>
        /// <param name="a_max">The maximum value inclusive</param>
        /// <returns>A random value in the range</returns>
        public static IVector4 Range(IVector4 a_min, IVector4 a_max)
        {
            return new IVector4(Range(a_min.X, a_max.X), Range(a_min.Y, a_max.Y), Range(a_min.Z, a_max.Z), Range(a_min.W, a_max.W));
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