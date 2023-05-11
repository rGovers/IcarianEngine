using System;
using System.Runtime.InteropServices;
using System.Xml;

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
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Vector4
    {
        [FieldOffset(0)]
        public float X;
        [FieldOffset(4)]
        public float Y;
        [FieldOffset(8)]
        public float Z;
        [FieldOffset(12)]
        public float W;

#region CONSTANTS
        public static readonly Vector4 One = new Vector4(1.0f);
        public static readonly Vector4 Zero = new Vector4(0.0f);

        public static readonly Vector4 ZeroP = new Vector4(Vector3.Zero, 1.0f);

        public static readonly Vector4 Infinity = new Vector4(float.PositiveInfinity);

        public static readonly Vector4 UnitX = new Vector4(1.0f, 0.0f, 0.0f, 0.0f);
        public static readonly Vector4 UnitY = new Vector4(0.0f, 1.0f, 0.0f, 0.0f);
        public static readonly Vector4 UnitZ = new Vector4(0.0f, 0.0f, 1.0f, 0.0f);
        public static readonly Vector4 UnitW = new Vector4(0.0f, 0.0f, 0.0f, 1.0f);
#endregion

#region SWIZZLE
#region VECTOR2
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
        public Vector2 XW
        {
            get
            {
                return new Vector2(X, W);
            }
            set
            {
                X = value.X;
                Y = value.Y;
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
        public Vector2 YW
        {
            get
            {
                return new Vector2(Y, W);
            }
            set
            {
                Y = value.X;
                W = value.Y;
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
        public Vector2 ZW
        {
            get
            {
                return new Vector2(Z, W);
            }
            set
            {
                Z = value.X;
                W = value.Y;
            }
        }
        public Vector2 WX
        {
            get
            {
                return new Vector2(W, X);
            }
            set
            {
                W = value.X;
                X = value.Y;
            }
        }
        public Vector2 WY
        {
            get
            {
                return new Vector2(W, Y);
            }
            set
            {
                W = value.X;
                Y = value.Y;
            }
        }
        public Vector2 WZ
        {
            get
            {
                return new Vector2(W, Z);
            }
            set
            {
                W = value.X;
                Z = value.Y;
            }
        }
        public Vector2 WW
        {
            get
            {
                return new Vector2(W);
            }
        }
#endregion

#region VECTOR3
#region X
        public Vector3 XXX
        {
            get
            {
                return new Vector3(X);
            }
        }
        public Vector3 XXY
        {
            get
            {
                return new Vector3(XX, Y);
            }
        }
        public Vector3 XXZ
        {
            get
            {
                return new Vector3(XX, Z);
            }
        }
        public Vector3 XXW
        {
            get
            {
                return new Vector3(XX, W);
            }
        }
        public Vector3 XYX
        {
            get
            {
                return new Vector3(XY, X);
            }
        }
        public Vector3 XYY
        {
            get
            {
                return new Vector3(XY, Y);
            }
        }
        public Vector3 XYZ
        {
            get
            {
                return new Vector3(XY, Z);
            }
            set
            {
                XY = value.XY;
                Z = value.Z;
            }
        }
        public Vector3 XYW
        {
            get
            {
                return new Vector3(XY, W);
            }
            set
            {
                XY = value.XY;
                W = value.Z;
            }
        }
        public Vector3 XZX
        {
            get
            {
                return new Vector3(XZ, X);
            }
        }
        public Vector3 XZY
        {
            get
            {
                return new Vector3(XZ, Y);
            }
            set
            {
                XZ = value.XY;
                Y = value.Z;
            }
        }
        public Vector3 XZZ
        {
            get
            {
                return new Vector3(XZ, Z);
            }
        }
        public Vector3 XZW
        {
            get
            {
                return new Vector3(XZ, W);
            }
            set
            {
                XZ = value.XY;
                W = value.Z;
            }
        }
        public Vector3 XWX
        {
            get
            {
                return new Vector3(XW, X);
            }
        }
        public Vector3 XWY
        {
            get
            {
                return new Vector3(XW, Y);
            }
            set
            {
                XW = value.XY;
                Y = value.Z;
            }
        }
        public Vector3 XWZ
        {
            get
            {
                return new Vector3(XW, Z);
            }
            set
            {
                XW = value.XY;
                Z = value.Z;
            }
        }
        public Vector3 XWW
        {
            get
            {
                return new Vector3(XW, W);
            }
        }
#endregion

#region Y
        public Vector3 YXX
        {
            get
            {
                return new Vector3(YX, X);
            }
        }
        public Vector3 YXY
        {
            get
            {
                return new Vector3(YX, Y);
            }
        }
        public Vector3 YXZ
        {
            get
            {
                return new Vector3(YX, Z);
            }
            set
            {
                YX = value.XY;
                Z = value.Z;
            }
        }
        public Vector3 YXW
        {
            get
            {
                return new Vector3(YX, W);
            }
            set
            {
                YX = value.XY;
                W = value.Z;
            }
        }
        public Vector3 YYX
        {
            get
            {
                return new Vector3(YY, X);
            }
        }
        public Vector3 YYY
        {
            get
            {
                return new Vector3(YY, Y);
            }
        }
        public Vector3 YYZ
        {
            get
            {
                return new Vector3(YY, Z);
            }
        }
        public Vector3 YYW
        {
            get
            {
                return new Vector3(YY, W);
            }
        }
        public Vector3 YZX
        {
            get
            {
                return new Vector3(YZ, X);
            }
            set
            {
                YZ = value.XY;
                X = value.Z;
            }
        }
        public Vector3 YZY
        {
            get
            {
                return new Vector3(YZ, Y);
            }
        }
        public Vector3 YZZ
        {
            get
            {
                return new Vector3(YZ, Z);
            }
        }
        public Vector3 YZW
        {
            get
            {
                return new Vector3(YZ, W);
            }
            set
            {
                YZ = value.XY;
                W = value.Z;
            }
        }
        public Vector3 YWX
        {
            get
            {
                return new Vector3(YW, X);
            }
            set
            {
                YX = value.XY;
                X = value.Z;
            }
        }
        public Vector3 YWY
        {
            get
            {
                return new Vector3(YW, Y);
            }
        }
        public Vector3 YWZ
        {
            get
            {
                return new Vector3(YW, Z);
            }
            set
            {
                YW = value.XY;
                Z = value.Z;
            }
        }
        public Vector3 YWW
        {
            get
            {
                return new Vector3(YW, W);
            }
        }
#endregion

#region Z
    public Vector3 ZXX
    {
        get
        {
            return new Vector3(ZX, X);
        }
    }
    public Vector3 ZXY
    {
        get
        {
            return new Vector3(ZX, Y);
        }
        set
        {
            ZX = value.XY;
            Y = value.Z;
        }
    }
    public Vector3 ZXZ
    {
        get
        {
            return new Vector3(ZX, Z);
        }
    }
    public Vector3 ZXW
    {
        get
        {
            return new Vector3(ZX, W);
        }
        set
        {
            ZX = value.XY;
            W = value.Z;
        }
    }
    public Vector3 ZYX
    {
        get
        {
            return new Vector3(ZY, X);
        }
        set
        {
            ZY = value.XY;
            X = value.Z;
        }
    }
    public Vector3 ZYY
    {
        get
        {
            return new Vector3(ZY, Y);
        }
    }
    public Vector3 ZYZ
    {
        get
        {
            return new Vector3(ZY, Z);
        }
    }
    public Vector3 ZYW
    {
        get
        {
            return new Vector3(ZY, W);
        }
        set
        {
            ZY = value.XY;
            W = value.Z;
        }
    }
    public Vector3 ZZX
    {
        get
        {
            return new Vector3(ZZ, X);
        }
    }
    public Vector3 ZZY
    {
        get
        {
            return new Vector3(ZZ, Y);
        }
    }
    public Vector3 ZZZ
    {
        get
        {
            return new Vector3(Z);
        }
    }
    public Vector3 ZZW
    {
        get
        {
            return new Vector3(ZZ, W);
        }
    }
    public Vector3 ZWX
    {
        get
        {
            return new Vector3(ZW, X);
        }
        set
        {
            ZW = value.XY;
            X = value.Z;
        }
    }
    public Vector3 ZWY
    {
        get
        {
            return new Vector3(ZW, Y);
        }
        set
        {
            ZW = value.XY;
            Y = value.Z;
        }
    }
    public Vector3 ZWZ
    {
        get
        {
            return new Vector3(ZW, Z);
        }
    }
    public Vector3 ZWW
    {
        get
        {
            return new Vector3(ZW, W);
        }
    }
#endregion

#region W
    public Vector3 WXX
    {
        get
        {
            return new Vector3(WX, X);
        }
    }
    public Vector3 WXY
    {
        get
        {
            return new Vector3(WX, Y);
        }
        set
        {
            WX = value.XY;
            Y = value.Z;
        }
    }
    public Vector3 WXZ
    {
        get
        {
            return new Vector3(WX, Z);
        }
        set 
        {
            WX = value.XY;
            Z = value.Z;
        }
    }
    public Vector3 WYX
    {
        get
        {
            return new Vector3(WY, X);
        }
        set
        {
            WY = value.XY;
            X = value.Z;
        }
    }
    public Vector3 WYY 
    {
        get
        {
            return new Vector3(WY, Y);
        }
    }
    public Vector3 WYZ
    {
        get
        {
            return new Vector3(WY, Z);
        }
        set
        {
            WY = value.XY;
            Z = value.Z;
        }
    }
    public Vector3 WYW 
    {
        get
        {
            return new Vector3(WY, W);
        }
    }
    public Vector3 WZX
    {
        get
        {
            return new Vector3(WZ, X);
        }
        set
        {
            WZ = value.XY;
            X = value.Z;
        }
    }
    public Vector3 WZY
    {
        get
        {
            return new Vector3(WZ, Y);
        }
        set
        {
            WZ = value.XY;
            Y = value.Z;
        }
    }
    public Vector3 WZZ
    {
        get
        {
            return new Vector3(WZ, Z);
        }
    }
    public Vector3 WZW
    {
        get
        {
            return new Vector3(WX, W);
        }
    }
    public Vector3 WWX
    {
        get
        {
            return new Vector3(WW, X);
        }
    }
    public Vector3 WWY
    {
        get
        {
            return new Vector3(WW, Y);
        }
    }
    public Vector3 WWZ
    {
        get
        {
            return new Vector3(WW, Z);
        }
    }
    public Vector3 WWW
    {
        get
        {
            return new Vector3(WW, W);
        }
    }
#endregion
#endregion
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

        public Vector4(float a_val)
        {
            X = a_val;
            Y = a_val;
            Z = a_val;
            W = a_val;
        }
        public Vector4(float a_x, float a_y, float a_z, float a_w)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
            W = a_w;
        }
        public Vector4(Vector2 a_xy, float a_z, float a_w)
        {
            X = a_xy.X;
            Y = a_xy.Y;
            Z = a_z;
            W = a_w;
        }
        public Vector4(float a_x, Vector2 a_yz, float a_w)
        {
            X = a_x;
            Y = a_yz.X;
            Z = a_yz.Y;
            W = a_w;
        }
        public Vector4(float a_x, float a_y, Vector2 a_zw)
        {
            X = a_x;
            Y = a_y;
            Z = a_zw.X;
            W = a_zw.Y;
        }
        public Vector4(Vector2 a_xy, Vector2 a_zw)
        {
            X = a_xy.X;
            Y = a_xy.Y;
            Z = a_zw.X;
            W = a_zw.Y;
        }
        public Vector4(Vector3 a_xyz, float a_w)
        {
            X = a_xyz.X;
            Y = a_xyz.Y;
            Z = a_xyz.Z;
            W = a_w;
        }
        public Vector4(float a_x, Vector3 a_yzw)
        {
            X = a_x;
            Y = a_yzw.X;
            Z = a_yzw.Y;
            W = a_yzw.Z;
        }
        public Vector4(Vector4 a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }

        public Color ToColor()
        {
            return new Color((byte)(X * 255.0f), (byte)(Y * 255.0f), (byte)(Z * 255.0f), (byte)(W * 255.0f));
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

        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y + Z * Z + W * W;
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
            W /= mag;
        }
        public static Vector4 Normalized(Vector4 a_vec)
        {
            float mag = a_vec.Magnitude;

            return new Vector4(a_vec.X / mag, a_vec.Y / mag, a_vec.Z / mag, a_vec.W / mag);
        }

        public Quaternion ToQuaternion()
        {
            return new Quaternion(X, Y, Z, W);
        }
    }
}