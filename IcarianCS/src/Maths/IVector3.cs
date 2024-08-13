// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

namespace IcarianEngine.Maths
{
    public static class IVector3Extensions
    {
        public static IVector3 ToIVector3(this XmlElement a_element)
        {
            return ToIVector3(a_element, IVector3.Zero);
        }
        public static IVector3 ToIVector3(this XmlElement a_element, IVector3 a_default)
        {
            IVector3 vec = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                case "U":
                case "R":
                {
                    vec.X = int.Parse(element.InnerText);

                    break;
                }
                case "Y":
                case "V":
                case "G":
                {
                    vec.Y = int.Parse(element.InnerText);

                    break;
                }
                case "Z":
                case "W":
                case "B":
                {
                    vec.Z = int.Parse(element.InnerText);

                    break;
                }
                }
            }

            return vec;
        }

        public static XmlElement ToXml(this IVector3 a_vec, XmlDocument a_doc, string a_name)
        {
            return ToXml(a_vec, a_doc, a_name, IVector3.Zero);
        }
        public static XmlElement ToXml(this IVector3 a_vec, XmlDocument a_doc, string a_name, IVector3 a_default)
        {
            if (a_vec == a_default)
            {
                return null;
            }

            XmlElement element = a_doc.CreateElement(a_name);

            if (a_vec.X != a_default.X)
            {
                XmlElement x = a_doc.CreateElement("X");
                x.InnerText = a_vec.X.ToString();
                element.AppendChild(x);
            }
            if (a_vec.Y != a_default.Y)
            {
                XmlElement y = a_doc.CreateElement("Y");
                y.InnerText = a_vec.Y.ToString();
                element.AppendChild(y);
            }
            if (a_vec.Z != a_default.Z)
            {
                XmlElement z = a_doc.CreateElement("Z");
                z.InnerText = a_vec.Z.ToString();
                element.AppendChild(z);
            }

            return element;
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct IVector3
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
        /// The Z component of the vector
        /// </summary>
        [FieldOffset(8)]
        public int Z;

        /// <summary>
        /// Zero vector
        /// </summary>
        public static readonly IVector3 Zero = new IVector3(0);
        /// <summary>
        /// One vector
        /// </summary>
        public static readonly IVector3 One = new IVector3(1);
        
        /// <summary>
        /// Right vector
        /// </summary>
        public static readonly IVector3 Right = new IVector3(1, 0, 0);
        /// <summary>
        /// Left vector
        /// </summary>
        public static readonly IVector3 Left = new IVector3(-1, 0, 0);
        /// <summary>
        /// Up vector
        /// </summary>
        public static readonly IVector3 Up = new IVector3(0, -1, 0);
        /// <summary>
        /// Down vector
        /// </summary>
        public static readonly IVector3 Down = new IVector3(0, 1, 0);
        /// <summary>
        /// Forward vector
        /// </summary>
        public static readonly IVector3 Forward = new IVector3(0, 0, -1);
        /// <summary>
        /// Backward vector
        /// </summary>
        public static readonly IVector3 Backward = new IVector3(0, 0, 1);

        /// <summary>
        /// Unit X vector
        /// </summary>
        public static readonly IVector3 UnitX = new IVector3(1, 0, 0);
        /// <summary>
        /// Unit Y vector
        /// </summary>
        public static readonly IVector3 UnitY = new IVector3(0, 1, 0);
        /// <summary>
        /// Unit Z vector
        /// </summary>
        public static readonly IVector3 UnitZ = new IVector3(0, 0, 1);

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
        /// W Component. Maps to the Z component of the vector
        /// </summary>
        public int W
        {
            get
            {
                return Z;
            }
            set
            {
                Z = value;
            }
        }

        /// <summary>
        /// R Component. Maps to the X component of the vector
        /// </summary>
        public int R
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
        /// G Component. Maps to the Y component of the vector
        /// </summary>
        public int G
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
        /// B Component. Maps to the Z component of the vector
        /// </summary>
        public int B
        {
            get
            {
                return Z;
            }
            set
            {
                Z = value;
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
                case 2:
                {
                    return Z;
                }
                default:
                {
                    Logger.IcarianError("Invalid Vector3 index");

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
                case 2:
                {
                    Z = value;

                    break;
                }
                default:
                {
                    Logger.IcarianError("Invalid Vector3 index");

                    break;
                }
                }
            }
        }

        /// <summary>
        /// The squared magnitude of the vector
        /// </summary>
        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y + Z * Z;
            }
        }

        /// <summary>
        /// The magnitude of the vector
        /// </summary>
        public float Magnitude
        {
            get
            {
                return Mathf.Sqrt(MagnitudeSqr);
            }
        }

        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_val">The value for all components</param>
        public IVector3(int a_val)
        {
            X = a_val;
            Y = a_val;   
            Z = a_val;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">The X component of the vector</param>
        /// <param name="a_y">The Y component of the vector</param>
        /// <param name="a_z">The Z component of the vector</param>
        public IVector3(int a_x, int a_y, int a_z)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_xy">The XY components of the vector</param>
        /// <param name="a_z">The Z component of the vector</param>
        public IVector3(IVector3 a_xy, int a_z)
        {
            X = a_xy.X;
            Y = a_xy.Y;
            Z = a_z;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">The X component of the vector</param>
        /// <param name="a_yz">The YZ components of the vector</param>
        public IVector3(int a_x, IVector2 a_yz)
        {
            X = a_x;
            Y = a_yz.X;
            Z = a_yz.Y;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_xyz">Vector to copy</param>
        public IVector3(IVector3 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
        }

        public static explicit operator IVector3(Vector3 a_vec)
        {
            return new IVector3((int)a_vec.X, (int)a_vec.Y, (int)a_vec.Z);
        }

        public static IVector3 operator -(IVector3 a_vec)
        {
            return new IVector3(-a_vec.X, -a_vec.Y, -a_vec.Z);
        }
        
        public static IVector3 operator +(IVector3 a_lhs, IVector3 a_rhs)
        {
            return new IVector3(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y, a_lhs.Z + a_rhs.Z);
        }
        public static IVector3 operator -(IVector3 a_lhs, IVector3 a_rhs)
        {
            return new IVector3(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y, a_lhs.Z - a_rhs.Z);   
        }
        public static IVector3 operator *(IVector3 a_lhs, float a_rhs)
        {
            return new IVector3((int)(a_lhs.X * a_rhs), (int)(a_lhs.Y * a_rhs), (int)(a_lhs.Z * a_rhs));
        }
        public static IVector3 operator /(IVector3 a_lhs, float a_rhs)
        {
            return new IVector3((int)(a_lhs.X / a_rhs), (int)(a_lhs.Y / a_rhs), (int)(a_lhs.Z / a_rhs));
        }
        public static IVector3 operator *(IVector3 a_lhs, IVector3 a_rhs)
        {
            return new IVector3(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y, a_lhs.Z * a_rhs.Z);
        }
        public static IVector3 operator /(IVector3 a_lhs, IVector3 a_rhs)
        {
            return new IVector3(a_lhs.X / a_rhs.X, a_lhs.Y / a_rhs.Y, a_lhs.Z / a_rhs.Z);
        }

        public static bool operator ==(IVector3 a_lhs, IVector3 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y && a_lhs.Z == a_rhs.Z;
        }
        public static bool operator !=(IVector3 a_lhs, IVector3 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y || a_lhs.Z != a_rhs.Z;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            IVector3 vec = (IVector3)a_obj;

            return X == vec.X && Y == vec.Y && Z == vec.Z;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // Lazy so just prime hash till it bites me
                int hash = 73;
                hash = hash * 79 + X.GetHashCode();
                hash = hash * 79 + Y.GetHashCode();
                hash = hash * 79 + Z.GetHashCode();
                return hash;
            }
        }
        public override string ToString()
        {
            return $"({X}, {Y}, {Z})";
        }

        /// <summary>
        /// Gets the dot product of the two vectors
        /// </summary>
        /// <param name="a_lhs">Left hand side of the dot product</param>
        /// <param name="a_rhs">Right hand side of the dot product</param>
        /// <returns>Dot product of the two vectors</returns>
        public static float Dot(IVector3 a_lhs, IVector3 a_rhs)
        {
            return a_lhs.X * a_rhs.X + a_lhs.Y * a_rhs.Y + a_lhs.Z * a_rhs.Z;
        }
        /// <summary>
        /// Gets the cross product of the two vectors
        /// </summary>
        /// <param name="a_lhs">Left hand side of the cross product</param>
        /// <param name="a_rhs">Right hand side of the cross product</param>
        /// <returns>Cross product of the two vectors</returns>
        public static IVector3 Cross(IVector3 a_lhs, IVector3 a_rhs)
        {
            return new IVector3
            (
                a_lhs.Y * a_rhs.Z - a_lhs.Z * a_rhs.Y,
                a_lhs.Z * a_rhs.X - a_lhs.X * a_rhs.Z,
                a_lhs.X * a_rhs.Y - a_lhs.Y * a_rhs.X
            );
        }

        /// <summary>
        /// Linearly interpolates between two vectors
        /// </summary>
        /// <param name="a_start">Start vector</param>
        /// <param name="a_end">End vector</param>
        /// <param name="a_t">Interpolation value</param>
        /// <returns>Interpolated vector</returns>
        public static IVector3 Lerp(IVector3 a_start, IVector3 a_end, float a_t)
        {
            return a_start + (a_end - a_start) * a_t;
        }

        VEC_SWIZZLE_IVEC3_FULL_VEC2
        VEC_SWIZZLE_IVEC3_FULL_VEC3
        VEC_SWIZZLE_IVEC3_FULL_VEC4
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