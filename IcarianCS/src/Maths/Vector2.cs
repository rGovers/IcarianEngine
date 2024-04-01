using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

namespace IcarianEngine.Maths
{
    public static class Vector2Extensions
    {
        /// <summary>
        /// Converts a XmlElement to a Vector2
        /// </summary>
        public static Vector2 ToVector2(this XmlElement a_element)
        {
            return ToVector2(a_element, Vector2.Zero);
        }
        /// <summary>
        /// Convertex a XmlElement to a Vector2 with a default value
        /// </summary>
        public static Vector2 ToVector2(this XmlElement a_element, Vector2 a_default)
        {
            Vector2 vec = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                case "S":
                case "U":
                {
                    vec.X = float.Parse(element.InnerText);

                    break;
                }
                case "Y":
                case "T":
                case "V":
                {
                    vec.Y = float.Parse(element.InnerText);

                    break;
                }
                }
            }

            return vec;
        }

        /// <summary>
        /// Creates an XmlElement from a Vector2
        /// </summary>
        public static XmlElement ToXml(this Vector2 a_vec, XmlDocument a_doc, string a_name)
        {
            return ToXml(a_vec, a_doc, a_name, Vector2.Zero);
        }
        /// <summary>
        /// Creates an XmlElement from a Vector2
        /// </summary>
        public static XmlElement ToXml(this Vector2 a_vec, XmlDocument a_doc, string a_name, Vector2 a_default)
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
    public struct Vector2
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
        /// Zero vector
        /// </summary>
        public static readonly Vector2 Zero = new Vector2(0.0f);
        /// <summary>
        /// One vector
        /// </summary>
        public static readonly Vector2 One = new Vector2(1.0f);

        /// <summary>
        /// Infinity vector
        /// </summary>
        public static readonly Vector2 Infinity = new Vector2(float.PositiveInfinity);

        /// <summary>
        /// Right vector
        /// </summary>
        public static readonly Vector2 Right = new Vector2(1.0f, 0.0f);
        /// <summary>
        /// Left vector
        /// </summary>
        public static readonly Vector2 Left = new Vector2(-1.0f, 0.0f);
        /// <summary>
        /// Up vector
        /// </summary>
        public static readonly Vector2 Up = new Vector2(0.0f, 1.0f);
        /// <summary>
        /// Down vector
        /// </summary>
        public static readonly Vector2 Down = new Vector2(0.0f, -1.0f);

        /// <summary>
        /// Unit X vector
        /// </summary>
        public static readonly Vector2 UnitX = new Vector2(1.0f, 0.0f);
        /// <summary>
        /// Unit Y vector
        /// </summary>
        public static readonly Vector2 UnitY = new Vector2(0.0f, 1.0f);

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
        /// S Component. Maps to the X component of the vector
        /// </summary>
        public float S
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
        public float T
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
                default:
                {
                    Logger.IcarianError("Invalid Vector2 index");

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
                default:
                {
                    Logger.IcarianError("Invalid Vector2 index");

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
                return (float)Math.Sqrt(X * X + Y * Y);
            }
        }

        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_val">Value to set all components to</param>
        public Vector2(float a_val)
        {
            X = a_val;
            Y = a_val;   
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_x">X component of the vector</param>
        /// <param name="a_y">Y component of the vector</param>
        public Vector2(float a_x, float a_y)
        {
            X = a_x;
            Y = a_y;
        }
        /// <summary>
        /// Constructor for the vector
        /// </summary>
        /// <param name="a_other">Vector to copy</param>
        public Vector2(Vector2 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
        }

        public static explicit operator Vector2(IVector2 a_vec)
        {
            return new Vector2(a_vec.X, a_vec.Y);
        }

        public static Vector2 operator -(Vector2 a_vec)
        {
            return new Vector2(-a_vec.X, -a_vec.Y);
        }

        public static Vector2 operator +(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y);
        }
        public static Vector2 operator -(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y);
        }
        public static Vector2 operator *(Vector2 a_lhs, float a_rhs)
        {
            return new Vector2(a_lhs.X * a_rhs, a_lhs.Y * a_rhs);
        }
        public static Vector2 operator /(Vector2 a_lhs, float a_rhs)
        {
            return new Vector2(a_lhs.X / a_rhs, a_lhs.Y / a_rhs);
        }
        public static Vector2 operator %(Vector2 a_lhs, float a_rhs)
        {
            return new Vector2(a_lhs.X % a_rhs, a_lhs.Y % a_rhs);
        }
        public static Vector2 operator *(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X * a_rhs.X, a_lhs.Y * a_rhs.Y);
        }
        public static Vector2 operator /(Vector2 a_lhs, Vector2 a_rhs)
        {
            return new Vector2(a_lhs.X / a_rhs.X, a_lhs.Y / a_rhs.Y);
        }        

        public static bool operator ==(Vector2 a_lhs, Vector2 a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y;
        }
        public static bool operator !=(Vector2 a_lhs, Vector2 a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            Vector2 vec = (Vector2)a_obj;

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
        /// Normalizes the vector
        /// </summary>
        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
        }
        
        /// <summary>
        /// Gets a normalized copy of the vector
        /// </summary>
        /// <param name="a_vector">Vector to normalize</param>
        /// <returns>Normalized vector</returns>
        public static Vector2 Normalized(Vector2 a_vector)
        {
            float mag = a_vector.Magnitude;

            return new Vector2(a_vector.X / mag, a_vector.Y / mag);
        }

        /// <summary>
        /// Gets the dot product of two vectors
        /// </summary>
        /// <param name="a_lhs">Left hand side of the dot product</param>
        /// <param name="a_rhs">Right hand side of the dot product</param>
        /// <returns>Dot product of the two vectors</returns>
        public static float Dot(Vector2 a_lhs, Vector2 a_rhs)
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
        public static Vector2 Lerp(Vector2 a_start, Vector2 a_end, float a_t)
        {
            return a_start + (a_end - a_start) * a_t;
        }

        /// @cond SWIZZLE

        VEC_SWIZZLE_VEC2_FULL_VEC2
        VEC_SWIZZLE_VEC2_FULL_VEC3
        VEC_SWIZZLE_VEC2_FULL_VEC4
        
        /// @endcond
    }
}