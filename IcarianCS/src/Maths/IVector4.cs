using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

namespace IcarianEngine.Maths
{
    public static class IVector4Extensions
    {
        public static IVector4 ToIVector4(this XmlElement a_element)
        {
            return ToIVector4(a_element, IVector4.Zero);
        }
        public static IVector4 ToIVector4(this XmlElement a_element, IVector4 a_default)
        {
            IVector4 vec = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                case "R":
                {
                    vec.X = int.Parse(element.InnerText);

                    break;
                }
                case "Y":
                case "G":
                {
                    vec.Y = int.Parse(element.InnerText);

                    break;
                }
                case "Z":
                case "B":
                {
                    vec.Z = int.Parse(element.InnerText);

                    break;
                }
                case "W":
                case "A":
                {
                    vec.W = int.Parse(element.InnerText);

                    break;
                }
                }
            }

            return vec;
        }

        public static XmlElement ToXml(this IVector4 a_vec, XmlDocument a_doc, string a_name)
        {
            return ToXml(a_vec, a_doc, a_name, IVector4.Zero);
        }
        public static XmlElement ToXml(this IVector4 a_vec, XmlDocument a_doc, string a_name, IVector4 a_default)
        {
            XmlElement element = a_doc.CreateElement(a_name);

            if (a_vec != a_default)
            {
                XmlElement x = a_doc.CreateElement("X");
                x.InnerText = a_vec.X.ToString();
                element.AppendChild(x);

                XmlElement y = a_doc.CreateElement("Y");
                y.InnerText = a_vec.Y.ToString();
                element.AppendChild(y);

                XmlElement z = a_doc.CreateElement("Z");
                z.InnerText = a_vec.Z.ToString();
                element.AppendChild(z);

                XmlElement w = a_doc.CreateElement("W");
                w.InnerText = a_vec.W.ToString();
                element.AppendChild(w);
            }

            return element;
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct IVector4
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
        /// The W component of the vector
        /// </summary>
        [FieldOffset(12)]
        public int W;

        /// <summary>
        /// Zero vector
        /// </summary>
        public static readonly IVector4 Zero = new IVector4(0);
        /// <summary>
        /// One vector
        /// </summary>
        public static readonly IVector4 One = new IVector4(1);

        /// <summary>
        /// Zero position vector
        /// </summary>
        public static readonly IVector4 ZeroP = new IVector4(IVector3.Zero, 1);

        /// <summary>
        /// UnitX vector
        /// </summary>
        public static readonly IVector4 UnitX = new IVector4(1, 0, 0, 0);
        /// <summary>
        /// UnitY vector
        /// </summary>
        public static readonly IVector4 UnitY = new IVector4(0, 1, 0, 0);
        /// <summary>
        /// UnitZ vector
        /// </summary>
        public static readonly IVector4 UnitZ = new IVector4(0, 0, 1, 0);
        /// <summary>
        /// UnitW vector
        /// </summary>
        public static readonly IVector4 UnitW = new IVector4(0, 0, 0, 1);
        
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
        /// A Component. Maps to the W component of the vector
        /// </summary>
        public int A
        {
            get
            {
                return W;
            }
            set
            {
                W = value;
            }
        }

        /// <summary>
        /// Indexer for the vector, int.MaxValue returned for invalid index
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
                case 3:
                {
                    return W;
                }
                default:
                {
                    Logger.IcarianError("Invalid Vector4 index");

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
                case 3:
                {
                    W = value;

                    break;
                }
                default:
                {
                    Logger.IcarianError("Invalid Vector4 index");

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
                return X * X + Y * Y + Z * Z + W * W;
            }
        }
        /// <summary>
        /// The magnitude of the vector
        /// </summary>
        public float Magnitude
        {
            get
            {
                return (float)Math.Sqrt(MagnitudeSqr);
            }
        }

        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_val">The value for all components</param>
        public IVector4(int a_val)
        {
            X = a_val;
            Y = a_val;
            Z = a_val;
            W = a_val;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">The X component of the vector</param>
        /// <param name="a_y">The Y component of the vector</param>
        /// <param name="a_z">The Z component of the vector</param>
        /// <param name="a_w">The W component of the vector</param>
        public IVector4(int a_x, int a_y, int a_z, int a_w)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
            W = a_w;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_xy">The XY components of the vector</param>
        /// <param name="a_z">The Z component of the vector</param>
        /// <param name="a_w">The W component of the vector</param>
        public IVector4(IVector2 a_xy, int a_z, int a_w)
        {
            X = a_xy.X;
            Y = a_xy.Y;
            Z = a_z;
            W = a_w;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">The X component of the vector</param>
        /// <param name="a_yz">The YZ components of the vector</param>
        /// <param name="a_w">The W component of the vector</param>
        public IVector4(int a_x, IVector2 a_yz, int a_w)
        {
            X = a_x;
            Y = a_yz.X;
            Z = a_yz.Y;
            W = a_w;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">The X component of the vector</param>
        /// <param name="a_y">The Y component of the vector</param>
        /// <param name="a_zw">The ZW components of the vector</param>
        public IVector4(int a_x, int a_y, IVector2 a_zw)
        {
            X = a_x;
            Y = a_y;
            Z = a_zw.X;
            W = a_zw.Y;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_xy">The XY components of the vector</param>
        /// <param name="a_zw">The ZW components of the vector</param>
        public IVector4(IVector2 a_xy, IVector2 a_zw)
        {
            X = a_xy.X;
            Y = a_xy.Y;
            Z = a_zw.X;
            W = a_zw.Y;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_xyz">The XYZ components of the vector</param>
        /// <param name="a_w">The W component of the vector</param>
        public IVector4(IVector3 a_xyz, int a_w)
        {
            X = a_xyz.X;
            Y = a_xyz.Y;
            Z = a_xyz.Z;
            W = a_w;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">The X component of the vector</param>
        /// <param name="a_yzw">The YZW components of the vector</param>
        public IVector4(int a_x, IVector3 a_yzw)
        {
            X = a_x;
            Y = a_yzw.X;
            Z = a_yzw.Y;
            W = a_yzw.Z;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_other">Vector to copy</param>
        public IVector4(IVector4 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }

        public static explicit operator IVector4(Vector4 a_vec)
        {
            return new IVector4((int)a_vec.X, (int)a_vec.Y, (int)a_vec.Z, (int)a_vec.W);
        }

        /// <summary>
        /// Converts the vector to a color
        /// </summary>
        /// <returns>The color</returns>
        public Color ToColor()
        {
            return new Color((byte)X, (byte)Y, (byte)Z, (byte)W);
        }

        public static IVector4 operator -(IVector4 a_vec)
        {
            return new IVector4(-a_vec.X, -a_vec.Y, -a_vec.Z, -a_vec.W);
        }

        public static IVector4 operator +(IVector4 a_lhs, IVector4 a_rhs)
        {
            return new IVector4(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y, a_lhs.Z + a_rhs.Z, a_lhs.W + a_rhs.W);
        }
        public static IVector4 operator -(IVector4 a_lhs, IVector4 a_rhs)
        {
            return new IVector4(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y, a_lhs.Z - a_rhs.Z, a_lhs.W - a_rhs.W);
        }
        public static IVector4 operator *(IVector4 a_lhs, float a_rhs)
        {
            return new IVector4((int)(a_lhs.X * a_rhs), (int)(a_lhs.Y * a_rhs), (int)(a_lhs.Z * a_rhs), (int)(a_lhs.W * a_rhs));
        }
        public static IVector4 operator /(IVector4 a_lhs, float a_rhs)
        {
            return new IVector4((int)(a_lhs.X / a_rhs), (int)(a_lhs.Y / a_rhs), (int)(a_lhs.Z / a_rhs), (int)(a_lhs.W / a_rhs));
        }
        public static IVector4 operator *(IVector4 a_lhs, IVector4 a_rhs)
        {
            return new IVector4(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y, a_lhs.Z * a_rhs.Z, a_lhs.W * a_rhs.W);
        }
        public static IVector4 operator /(IVector4 a_lhs, IVector4 a_rhs)
        {
            return new IVector4(a_lhs.X / a_rhs.X, a_lhs.Y / a_rhs.Y, a_lhs.Z / a_rhs.Y, a_lhs.W / a_rhs.W);
        }

        public static bool operator ==(IVector4 a_lhs, IVector4 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y && a_lhs.Z == a_rhs.Z && a_lhs.W == a_rhs.W;
        }
        public static bool operator !=(IVector4 a_lhs, IVector4 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y || a_lhs.Z != a_rhs.Z || a_lhs.W != a_rhs.W;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            IVector4 vec = (IVector4)a_obj;

            return X == vec.X && Y == vec.Y && Z == vec.Z && W == vec.W;
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
                hash = hash * 79 + W.GetHashCode();
                return hash;
            }
        }
        public override string ToString()
        {
            return $"({X}, {Y}, {Z}, {W})";
        }

        /// <summary>
        /// Linearly interpolates between two vectors
        /// </summary>
        /// <param name="a_start">The start vector</param>
        /// <param name="a_end">The end vector</param>
        /// <param name="a_t">The time between the two vectors</param>
        /// <returns>The interpolated vector</returns>
        public static IVector4 Lerp(IVector4 a_start, IVector4 a_end, float a_t)
        {
            return a_start + (a_end - a_start) * a_t;
        }

        VEC_SWIZZLE_IVEC4_FULL_VEC2
        VEC_SWIZZLE_IVEC4_FULL_VEC3
        VEC_SWIZZLE_IVEC4_FULL_VEC4
    }
}