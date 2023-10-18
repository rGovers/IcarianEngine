using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

namespace IcarianEngine.Maths
{
    public static class Vector3Extensions
    {
        public static Vector3 ToVector3(this XmlElement a_element)
        {
            return ToVector3(a_element, Vector3.Zero);
        }
        public static Vector3 ToVector3(this XmlElement a_element, Vector3 a_default)
        {
            Vector3 vec = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                case "S":
                {
                    vec.X = float.Parse(element.InnerText);

                    break;
                }
                case "Y":
                case "T":
                {
                    vec.Y = float.Parse(element.InnerText);

                    break;
                }
                case "Z":
                case "U":
                {
                    vec.Z = float.Parse(element.InnerText);

                    break;
                }
                }
            }

            return vec;
        }

        public static XmlElement ToXml(this Vector3 a_vec, XmlDocument a_doc, string a_name)
        {
            return ToXml(a_vec, a_doc, a_name, Vector3.Zero);
        }
        public static XmlElement ToXml(this Vector3 a_vec, XmlDocument a_doc, string a_name, Vector3 a_default)
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
    public struct Vector3
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
        /// Zero vector
        /// </summary>
        public static readonly Vector3 Zero = new Vector3(0.0f);
        /// <summary>
        /// One vector
        /// </summary>
        public static readonly Vector3 One = new Vector3(1.0f);

        /// <summary>
        /// Infinity vector
        /// </summary>
        public static readonly Vector3 Infinity = new Vector3(float.PositiveInfinity);
        
        /// <summary>
        /// Right vector
        /// </summary>
        public static readonly Vector3 Right = new Vector3(1.0f, 0.0f, 0.0f);
        /// <summary>
        /// Left vector
        /// </summary>
        public static readonly Vector3 Left = new Vector3(-1.0f, 0.0f, 0.0f);
        /// <summary>
        /// Up vector
        /// </summary>
        public static readonly Vector3 Up = new Vector3(0.0f, -1.0f, 0.0f);
        /// <summary>
        /// Down vector
        /// </summary>
        public static readonly Vector3 Down = new Vector3(0.0f, 1.0f, 0.0f);
        /// <summary>
        /// Forward vector
        /// </summary>
        public static readonly Vector3 Forward = new Vector3(0.0f, 0.0f, -1.0f);
        /// <summary>
        /// Backward vector
        /// </summary>
        public static readonly Vector3 Backward = new Vector3(0.0f, 0.0f, 1.0f);

        /// <summary>
        /// Unit X vector
        /// </summary>
        public static readonly Vector3 UnitX = new Vector3(1.0f, 0.0f, 0.0f);
        /// <summary>
        /// Unit Y vector
        /// </summary>
        public static readonly Vector3 UnitY = new Vector3(0.0f, 1.0f, 0.0f);
        /// <summary>
        /// Unit Z vector
        /// </summary>
        public static readonly Vector3 UnitZ = new Vector3(0.0f, 0.0f, 1.0f);

        /// <summary>
        /// U Component. Maps to the X component of the vector
        /// </summary>
        public float U
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
        public float V
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
        public float W
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
                default:
                {
                    Logger.IcarianError("Invalid Vector3 index");

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
                return (float)Math.Sqrt(MagnitudeSqr);
            }
        }

        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_val">The value for all components</param>
        public Vector3(float a_val)
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
        public Vector3(float a_x, float a_y, float a_z)
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
        public Vector3(Vector2 a_xy, float a_z)
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
        public Vector3(float a_x, Vector2 a_yz)
        {
            X = a_x;
            Y = a_yz.X;
            Z = a_yz.Y;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_xyz">Vector to copy</param>
        public Vector3(Vector3 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
        }

        public static Vector3 operator -(Vector3 a_vec)
        {
            return new Vector3(-a_vec.X, -a_vec.Y, -a_vec.Z);
        }
        
        public static Vector3 operator +(Vector3 a_lhs, Vector3 a_rhs)
        {
            return new Vector3(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y, a_lhs.Z + a_rhs.Z);
        }
        public static Vector3 operator -(Vector3 a_lhs, Vector3 a_rhs)
        {
            return new Vector3(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y, a_lhs.Z - a_rhs.Z);   
        }
        public static Vector3 operator *(Vector3 a_lhs, float a_rhs)
        {
            return new Vector3(a_lhs.X * a_rhs, a_lhs.Y * a_rhs, a_lhs.Z * a_rhs);
        }
        public static Vector3 operator /(Vector3 a_lhs, float a_rhs)
        {
            return new Vector3(a_lhs.X / a_rhs, a_lhs.Y / a_rhs, a_lhs.Z / a_rhs);
        }
        public static Vector3 operator *(Vector3 a_lhs, Vector3 a_rhs)
        {
            return new Vector3(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y, a_lhs.Z * a_rhs.Z);
        }
        public static Vector3 operator /(Vector3 a_lhs, Vector3 a_rhs)
        {
            return new Vector3(a_lhs.X / a_rhs.X, a_lhs.Y / a_rhs.Y, a_lhs.Z / a_rhs.Z);
        }

        public static bool operator ==(Vector3 a_lhs, Vector3 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y && a_lhs.Z == a_rhs.Z;
        }
        public static bool operator !=(Vector3 a_lhs, Vector3 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y || a_lhs.Z != a_rhs.Z;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            Vector3 vec = (Vector3)a_obj;

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
        /// Normalizes the vector
        /// </summary>
        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
            Z /= mag;
        }
        /// <summary>
        /// Gets a normalized copy of the vector
        /// </summary>
        /// <param name="a_other">Vector to normalize</param>
        /// <returns>Normalized copy of the vector</returns>
        public static Vector3 Normalized(Vector3 a_other)
        {
            float mag = a_other.Magnitude;

            return new Vector3(a_other.X / mag, a_other.Y / mag, a_other.Z / mag);
        }

        /// <summary>
        /// Gets the dot product of the two vectors
        /// </summary>
        /// <param name="a_lhs">Left hand side of the dot product</param>
        /// <param name="a_rhs">Right hand side of the dot product</param>
        /// <returns>Dot product of the two vectors</returns>
        public static float Dot(Vector3 a_lhs, Vector3 a_rhs)
        {
            return a_lhs.X * a_rhs.X + a_lhs.Y * a_rhs.Y + a_lhs.Z * a_rhs.Z;
        }
        /// <summary>
        /// Gets the cross product of the two vectors
        /// </summary>
        /// <param name="a_lhs">Left hand side of the cross product</param>
        /// <param name="a_rhs">Right hand side of the cross product</param>
        /// <returns>Cross product of the two vectors</returns>
        public static Vector3 Cross(Vector3 a_lhs, Vector3 a_rhs)
        {
            return new Vector3
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
        public static Vector3 Lerp(Vector3 a_start, Vector3 a_end, float a_t)
        {
            return a_start + (a_end - a_start) * a_t;
        }

        VEC_SWIZZLE_VEC3_FULL_VEC2
        VEC_SWIZZLE_VEC3_FULL_VEC3
        VEC_SWIZZLE_VEC3_FULL_VEC4
    }
}