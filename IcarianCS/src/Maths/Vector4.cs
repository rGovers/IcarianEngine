using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

namespace IcarianEngine.Maths
{
    public static class Vector4Extensions
    {
        public static Vector4 ToVector4(this XmlElement a_element)
        {
            return ToVector4(a_element, Vector4.Zero);
        }
        public static Vector4 ToVector4(this XmlElement a_element, Vector4 a_default)
        {
            Vector4 vec = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                case "R":
                {
                    vec.X = float.Parse(element.InnerText);

                    break;
                }
                case "Y":
                case "G":
                {
                    vec.Y = float.Parse(element.InnerText);

                    break;
                }
                case "Z":
                case "B":
                {
                    vec.Z = float.Parse(element.InnerText);

                    break;
                }
                case "W":
                case "A":
                {
                    vec.W = float.Parse(element.InnerText);

                    break;
                }
                }
            }

            return vec;
        }

        public static XmlElement ToXml(this Vector4 a_vec, XmlDocument a_doc, string a_name)
        {
            return ToXml(a_vec, a_doc, a_name, Vector4.Zero);
        }
        public static XmlElement ToXml(this Vector4 a_vec, XmlDocument a_doc, string a_name, Vector4 a_default)
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
    public struct Vector4
    {
        /// <summary>
        /// The X component of the vector
        /// </summary>
        [FieldOffset(0)]
        public float X;
        /// <summary>
        /// The Y component of the vector
        /// </summary>
        [FieldOffset(4)]
        public float Y;
        /// <summary>
        /// The Z component of the vector
        /// </summary>
        [FieldOffset(8)]
        public float Z;
        /// <summary>
        /// The W component of the vector
        /// </summary>
        [FieldOffset(12)]
        public float W;

        /// <summary>
        /// Zero vector
        /// </summary>
        public static readonly Vector4 Zero = new Vector4(0.0f);
        /// <summary>
        /// One vector
        /// </summary>
        public static readonly Vector4 One = new Vector4(1.0f);

        /// <summary>
        /// Zero position vector
        /// </summary>
        public static readonly Vector4 ZeroP = new Vector4(Vector3.Zero, 1.0f);

        /// <summary>
        /// Infinity vector
        /// </summary>
        public static readonly Vector4 Infinity = new Vector4(float.PositiveInfinity);

        /// <summary>
        /// UnitX vector
        /// </summary>
        public static readonly Vector4 UnitX = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        /// <summary>
        /// UnitY vector
        /// </summary>
        public static readonly Vector4 UnitY = new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        /// <summary>
        /// UnitZ vector
        /// </summary>
        public static readonly Vector4 UnitZ = new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        /// <summary>
        /// UnitW vector
        /// </summary>
        public static readonly Vector4 UnitW = new Vector4(0.0f, 0.0f, 0.0f, 1.0f);
        
        /// <summary>
        /// R Component. Maps to the X component of the vector
        /// </summary>
        public float R
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
        public float G
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
        public float B
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
        public float A
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
        /// Indexer for the vector, NaN if invalid index
        /// </summary>
        public float this[int a_key]
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

                return float.NaN;
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
        public Vector4(float a_val)
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
        public Vector4(float a_x, float a_y, float a_z, float a_w)
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
        public Vector4(Vector2 a_xy, float a_z, float a_w)
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
        public Vector4(float a_x, Vector2 a_yz, float a_w)
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
        public Vector4(float a_x, float a_y, Vector2 a_zw)
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
        public Vector4(Vector2 a_xy, Vector2 a_zw)
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
        public Vector4(Vector3 a_xyz, float a_w)
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
        public Vector4(float a_x, Vector3 a_yzw)
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
        public Vector4(Vector4 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }

        public static explicit operator Vector4(IVector4 a_vec)
        {
            return new Vector4(a_vec.X, a_vec.Y, a_vec.Z, a_vec.W);
        }

        /// <summary>
        /// Converts the vector to a color
        /// </summary>
        /// <returns>The color</returns>
        public Color ToColor()
        {
            return new Color((byte)(X * 255.0f), (byte)(Y * 255.0f), (byte)(Z * 255.0f), (byte)(W * 255.0f));
        }

        public static Vector4 operator -(Vector4 a_vec)
        {
            return new Vector4(-a_vec.X, -a_vec.Y, -a_vec.Z, -a_vec.W);
        }

        public static Vector4 operator +(Vector4 a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y, a_lhs.Z + a_rhs.Z, a_lhs.W + a_rhs.W);
        }
        public static Vector4 operator -(Vector4 a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y, a_lhs.Z - a_rhs.Z, a_lhs.W - a_rhs.W);
        }
        public static Vector4 operator *(Vector4 a_lhs, float a_rhs)
        {
            return new Vector4(a_lhs.X * a_rhs, a_lhs.Y * a_rhs, a_lhs.Z * a_rhs, a_lhs.W * a_rhs);
        }
        public static Vector4 operator /(Vector4 a_lhs, float a_rhs)
        {
            return new Vector4(a_lhs.X / a_rhs, a_lhs.Y / a_rhs, a_lhs.Z / a_rhs, a_lhs.W / a_rhs);
        }
        public static Vector4 operator *(Vector4 a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y, a_lhs.Z * a_rhs.Z, a_lhs.W * a_rhs.W);
        }
        public static Vector4 operator /(Vector4 a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs.X / a_rhs.X, a_lhs.Y / a_rhs.Y, a_lhs.Z / a_rhs.Y, a_lhs.W / a_rhs.W);
        }

        public static bool operator ==(Vector4 a_lhs, Vector4 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y && a_lhs.Z == a_rhs.Z && a_lhs.W == a_rhs.W;
        }
        public static bool operator !=(Vector4 a_lhs, Vector4 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y || a_lhs.Z != a_rhs.Z || a_lhs.W != a_rhs.W;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            Vector4 vec = (Vector4)a_obj;

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
        /// Normalizes the vector
        /// </summary>
        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
            Z /= mag;
            W /= mag;
        }
        /// <summary>
        /// Gets a normalized copy of the vector
        /// </summary>
        /// <param name="a_vec">The vector to normalize</param>
        /// <returns>The normalized vector</returns>
        public static Vector4 Normalized(Vector4 a_vec)
        {
            float mag = a_vec.Magnitude;

            return new Vector4(a_vec.X / mag, a_vec.Y / mag, a_vec.Z / mag, a_vec.W / mag);
        }

        /// <summary>
        /// Converts the vector to a quaternion
        /// </summary>
        /// <returns>The quaternion</returns>
        public Quaternion ToQuaternion()
        {
            return new Quaternion(X, Y, Z, W);
        }

        /// <summary>
        /// Linearly interpolates between two vectors
        /// </summary>
        /// <param name="a_start">The start vector</param>
        /// <param name="a_end">The end vector</param>
        /// <param name="a_t">The time between the two vectors</param>
        /// <returns>The interpolated vector</returns>
        public static Vector4 Lerp(Vector4 a_start, Vector4 a_end, float a_t)
        {
            return a_start + (a_end - a_start) * a_t;
        }

        VEC_SWIZZLE_VEC4_FULL_VEC2
        VEC_SWIZZLE_VEC4_FULL_VEC3
        VEC_SWIZZLE_VEC4_FULL_VEC4
    }
}