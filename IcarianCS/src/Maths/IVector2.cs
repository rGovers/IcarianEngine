// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

namespace IcarianEngine.Maths
{
    public static class IVector2Extensions
    {
        public static IVector2 ToIVector2(this XmlElement a_element)
        {
            return ToIVector2(a_element, IVector2.Zero);
        }
        public static IVector2 ToIVector2(this XmlElement a_element, IVector2 a_default)
        {
            IVector2 vec = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                case "S":
                {
                    vec.X = int.Parse(element.InnerText);

                    break;
                }
                case "Y":
                case "T":
                {
                    vec.Y = int.Parse(element.InnerText);

                    break;
                }
                }
            }

            return vec;
        }

        public static XmlElement ToXml(this IVector2 a_vec, XmlDocument a_doc, string a_name)
        {
            return ToXml(a_vec, a_doc, a_name, IVector2.Zero);
        }
        public static XmlElement ToXml(this IVector2 a_vec, XmlDocument a_doc, string a_name, IVector2 a_default)
        {
            if (a_vec == a_default)
            {
                return null;
            }

            XmlElement element = a_doc.CreateElement(a_name);

            if (a_vec.X != a_default.X)
            {
                XmlElement xElement = a_doc.CreateElement("X");
                xElement.InnerText = a_vec.X.ToString();
                element.AppendChild(xElement);
            }

            if (a_vec.Y != a_default.Y)
            {
                XmlElement yElement = a_doc.CreateElement("Y");
                yElement.InnerText = a_vec.Y.ToString();
                element.AppendChild(yElement);
            }

            return element;
        }       
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct IVector2
    {   
        /// <summary>
        /// The X component of the vector
        /// </summary>
        [FieldOffset(0)]
        public int X;
        /// <summary>
        /// The Y component of the vector
        /// </summary>
        [FieldOffset(4)]
        public int Y;

        /// <summary>
        /// Zero vector
        /// </summary>
        public static readonly IVector2 Zero = new IVector2(0);
        /// <summary>
        /// One vector
        /// </summary>
        public static readonly IVector2 One = new IVector2(1);

        /// <summary>
        /// Right vector
        /// </summary>
        public static readonly IVector2 Right = new IVector2(1, 0);
        /// <summary>
        /// Left vector
        /// </summary>
        public static readonly IVector2 Left = new IVector2(-1, 0);
        /// <summary>
        /// Up vector
        /// </summary>
        public static readonly IVector2 Up = new IVector2(0, 1);
        /// <summary>
        /// Down vector
        /// </summary>
        public static readonly IVector2 Down = new IVector2(0, -1);

        /// <summary>
        /// Unit X vector
        /// </summary>
        public static readonly IVector2 UnitX = new IVector2(1, 0);
        /// <summary>
        /// Unit Y vector
        /// </summary>
        public static readonly IVector2 UnitY = new IVector2(0, 1);

        /// <summary>
        /// U Component. Maps to the X component of the vector
        /// </summary>
        public int U
        {
            get
            {
                return X;
            }
            set
            {
                X = value;
            }
        }
        /// <summary>
        /// V Component. Maps to the Y component of the vector
        /// </summary>
        public int V
        {
            get
            {
                return Y;
            }
            set
            {
                Y = value;
            }
        }

        /// <summary>
        /// S Component. Maps to the X component of the vector
        /// </summary>
        public int S
        {
            get
            {
                return X;
            }
            set
            {
                X = value;
            }
        }
        /// <summary>
        /// T Component. Maps to the Y component of the vector
        /// </summary>
        public int T
        {
            get
            {
                return Y;
            }
            set
            {
                Y = value;
            }
        }

        /// <summary>
        /// Indexer for the vector, int.MaxValue is returned if the key is invalid
        /// </summary>
        public int this[int a_key]
        {
            get
            {
                switch (a_key)
                {
                case 0:
                {
                    return X;
                }
                case 1:
                {
                    return Y;
                }
                default:
                {
                    Logger.IcarianError("Invalid IVector2 index");

                    break;
                }
                }

                return int.MaxValue;
            }
            set
            {
                switch (a_key)
                {
                case 0:
                {
                    X = value;
                    
                    break;
                }
                case 1:
                {
                    Y = value;

                    break;
                }
                default:
                {
                    Logger.IcarianError("Invalid IVector2 index");

                    break;
                }
                }
            }
        }

        /// <summary>
        /// Magnitude squared of the vector
        /// </summary>
        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y;
            }
        }
        /// <summary>
        /// Magnitude of the vector
        /// </summary>
        public float Magnitude
        {
            get
            {
                return Mathf.Sqrt(X * X + Y * Y);
            }
        }

        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_val">Value to set all components to</param>
        public IVector2(int a_val)
        {
            X = a_val;
            Y = a_val;   
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">X component of the vector</param>
        /// <param name="a_y">Y component of the vector</param>
        public IVector2(int a_x, int a_y)
        {
            X = a_x;
            Y = a_y;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_other">Vector to copy</param>
        public IVector2(IVector2 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
        }

        public static explicit operator IVector2(Vector2 a_vec)
        {
            return new IVector2((int)a_vec.X, (int)a_vec.Y);
        }

        public static IVector2 operator -(IVector2 a_vec)
        {
            return new IVector2(-a_vec.X, -a_vec.Y);
        }

        public static IVector2 operator +(IVector2 a_lhs, IVector2 a_rhs)
        {
            return new IVector2(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y);
        }
        public static IVector2 operator -(IVector2 a_lhs, IVector2 a_rhs)
        {
            return new IVector2(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y);
        }
        public static IVector2 operator *(IVector2 a_lhs, float a_rhs)
        {
            return new IVector2((int)(a_lhs.X * a_rhs), (int)(a_lhs.Y * a_rhs));
        }
        public static IVector2 operator /(IVector2 a_lhs, float a_rhs)
        {
            return new IVector2((int)(a_lhs.X / a_rhs), (int)(a_lhs.Y / a_rhs));
        }
        public static IVector2 operator *(IVector2 a_lhs, IVector2 a_rhs)
        {
            return new IVector2(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y);
        }
        public static IVector2 operator /(IVector2 a_lhs, IVector2 a_rhs)
        {
            return new IVector2(a_lhs.X / a_rhs.X, a_lhs.Y / a_rhs.Y);
        }

        public static bool operator ==(IVector2 a_lhs, IVector2 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y;
        }
        public static bool operator !=(IVector2 a_lhs, IVector2 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            IVector2 vec = (IVector2)a_obj;

            return X == vec.X && Y == vec.Y;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // Lazy so just prime hash till it bites me
                int hash = 73;
                hash = hash * 79 + X.GetHashCode();
                hash = hash * 79 + Y.GetHashCode();
                return hash;
            }
        }
        public override string ToString()
        {
            return $"({X}, {Y})";
        }

        /// <summary>
        /// Gets the dot product of two vectors
        /// </summary>
        /// <param name="a_lhs">Left hand side of the dot product</param>
        /// <param name="a_rhs">Right hand side of the dot product</param>
        /// <returns>Dot product of the two vectors</returns>
        public static float Dot(IVector2 a_lhs, IVector2 a_rhs)
        {
            return a_lhs.X * a_rhs.X + a_lhs.Y * a_rhs.Y;
        }

        /// <summary>
        /// Linearly interpolates between two vectors
        /// </summary>
        /// <param name="a_start">Start vector</param>
        /// <param name="a_end">End vector</param>
        /// <param name="a_t">Interpolation value</param>
        /// <returns>Interpolated vector</returns>
        public static IVector2 Lerp(IVector2 a_start, IVector2 a_end, float a_t)
        {
            return a_start + (a_end - a_start) * a_t;
        }

        VEC_SWIZZLE_IVEC2_FULL_VEC2
        VEC_SWIZZLE_IVEC2_FULL_VEC3
        VEC_SWIZZLE_IVEC2_FULL_VEC4
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