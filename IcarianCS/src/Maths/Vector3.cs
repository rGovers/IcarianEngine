using System;
using System.Runtime.InteropServices;
using System.Xml;

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
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Vector3
    {
        [FieldOffset(0)]
        public float X;
        [FieldOffset(4)]
        public float Y;
        [FieldOffset(8)]
        public float Z;

#region CONSTANTS
        public static readonly Vector3 Zero = new Vector3(0.0f);
        public static readonly Vector3 One = new Vector3(1.0f);

        public static readonly Vector3 Infinity = new Vector3(float.PositiveInfinity);

        public static readonly Vector3 Right = new Vector3(1.0f, 0.0f, 0.0f);
        public static readonly Vector3 Left = new Vector3(-1.0f, 0.0f, 0.0f);
        public static readonly Vector3 Up = new Vector3(0.0f, -1.0f, 0.0f);
        public static readonly Vector3 Down = new Vector3(0.0f, 1.0f, 0.0f);
        public static readonly Vector3 Forward = new Vector3(0.0f, 0.0f, -1.0f);
        public static readonly Vector3 Backward = new Vector3(0.0f, 0.0f, 1.0f);

        public static readonly Vector3 UnitX = new Vector3(1.0f, 0.0f, 0.0f);
        public static readonly Vector3 UnitY = new Vector3(0.0f, 1.0f, 0.0f);
        public static readonly Vector3 UnitZ = new Vector3(0.0f, 0.0f, 1.0f);
#endregion

#region SWIZZLE
        public Vector2 XX
        {
            get
            {
                return new Vector2(X);
            }
        }
        public Vector2 XY
        {
            get
            {
                return new Vector2(X, Y);
            }
            set
            {
                X = value.X;
                Y = value.Y;
            }
        }
        public Vector2 XZ
        {
            get
            {
                return new Vector2(X, Z);
            }
            set
            {
                X = value.X;
                Z = value.Y;
            }
        }
        public Vector2 YX
        {
            get
            {
                return new Vector2(Y, X);
            }
            set
            {
                Y = value.X;
                X = value.Y;
            }
        }
        public Vector2 YY
        {
            get
            {
                return new Vector2(Y);
            }
        }
        public Vector2 YZ
        {
            get
            {
                return new Vector2(Y, Z);
            }
            set
            {
                Y = value.X;
                Z = value.Y;
            }
        }
        public Vector2 ZX
        {
            get
            {
                return new Vector2(Z, X);
            }
            set
            {
                Z = value.X;
                X = value.Y;
            }
        }
        public Vector2 ZY 
        {
            get
            {
                return new Vector2(Z, Y);
            }
            set
            {
                Z = value.X;
                Y = value.Y;
            }
        }
        public Vector2 ZZ
        {
            get
            {
                return new Vector2(Z);
            }
        }
#endregion

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

        public Vector3(float a_val)
        {
            X = a_val;
            Y = a_val;   
            Z = a_val;
        }
        public Vector3(float a_x, float a_y, float a_z)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
        }
        public Vector3(Vector2 a_xy, float a_z)
        {
            X = a_xy.X;
            Y = a_xy.Y;
            Z = a_z;
        }
        public Vector3(float a_x, Vector2 a_yz)
        {
            X = a_x;
            Y = a_yz.X;
            Z = a_yz.Y;
        }
        public Vector3(Vector3 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
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

        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y + Z * Z;
            }
        }

        public float Magnitude
        {
            get
            {
                return (float)Math.Sqrt(MagnitudeSqr);
            }
        }

        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
            Z /= mag;
        }
        public static Vector3 Normalized(Vector3 a_other)
        {
            float mag = a_other.Magnitude;

            return new Vector3(a_other.X / mag, a_other.Y / mag, a_other.Z / mag);
        }

        public static float Dot(Vector3 a_lhs, Vector3 a_rhs)
        {
            return a_lhs.X * a_rhs.X + a_lhs.Y * a_rhs.Y + a_lhs.Z * a_rhs.Z;
        }
        public static Vector3 Cross(Vector3 a_lhs, Vector3 a_rhs)
        {
            return new Vector3
            (
                a_lhs.Y * a_rhs.Z - a_lhs.Z * a_rhs.Y,
                a_lhs.Z * a_rhs.X - a_lhs.X * a_rhs.Z,
                a_lhs.X * a_rhs.Y - a_lhs.Y * a_rhs.X
            );
        }
    }
}