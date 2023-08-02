using System;
using System.Runtime.InteropServices;
using System.Xml;

namespace IcarianEngine.Maths
{
    public static class QuaternionExtensions
    {
        public static Quaternion ToQuaternion(this XmlElement a_element)
        {
            return ToQuaternion(a_element, Quaternion.Identity);
        }
        public static Quaternion ToQuaternion(this XmlElement a_element, Quaternion a_default)
        {
            Quaternion quat = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "X":
                {
                    quat.X = float.Parse(element.InnerText);

                    break;
                }
                case "Y":
                {
                    quat.Y = float.Parse(element.InnerText);

                    break;
                }
                case "Z":
                {
                    quat.Z = float.Parse(element.InnerText);

                    break;
                }
                case "W":
                {
                    quat.W = float.Parse(element.InnerText);

                    break;
                }
                }
            }

            return quat;
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Quaternion
    {
        [FieldOffset(0)]
        public float X;
        [FieldOffset(4)]
        public float Y;
        [FieldOffset(8)]
        public float Z;
        [FieldOffset(12)]
        public float W;

        public static readonly Quaternion Identity = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f); 

        public Quaternion(Quaternion a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }
        public Quaternion(float a_x, float a_y, float a_z, float a_w)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
            W = a_w;
        }

        public static Quaternion FromDirectionVectors(Vector3 a_right, Vector3 a_up, Vector3 a_forward)
        {
            float tr = a_right.X + a_up.Y + a_forward.Z;

            if (tr > 0)
            {
                float s = Mathf.Sqrt(1 + tr) * 2;

                return new Quaternion
                (
                    (a_forward.Y - a_up.Z) / s,
                    (a_right.Z - a_forward.X) / s,
                    (a_up.X - a_right.Y) / s,
                    s * 0.25f
                );
            }
            else if (a_right.X > a_up.Y && a_forward.X > a_forward.Z)
            {
                float s = Mathf.Sqrt(1 + a_right.X - a_up.Y - a_forward.Z) * 2;

                return new Quaternion
                (
                    s * 0.25f,
                    (a_right.Y + a_up.X) / s,
                    (a_right.Z + a_forward.X) / s,
                    (a_forward.Y - a_up.Z) / s
                );
            }
            else if (a_up.Y > a_forward.Z)
            {
                float s = Mathf.Sqrt(1 + a_up.Y - a_right.X - a_forward.Z) * 2;

                return new Quaternion
                (
                    (a_right.Y + a_up.X) / s,
                    s * 0.25f,
                    (a_up.Z + a_forward.Y) / s,
                    (a_right.Z - a_forward.X) / s
                );
            }

            float sV = Mathf.Sqrt(1 + a_forward.Z - a_right.X - a_up.Y) * 2;

            return new Quaternion
            (
                (a_right.Z + a_forward.X) / sV,
                (a_up.Z + a_forward.Y) / sV,
                sV * 0.25f,
                (a_up.X - a_right.Y) / sV
            );
        }
        public static Quaternion FromMatrix(Matrix4 a_mat)
        {
            return FromDirectionVectors(a_mat[0].XYZ, a_mat[1].XYZ, a_mat[2].XYZ);
        }
        public static Quaternion FromAxisAngle(Vector3 a_axis, float a_angle)
        {
            float halfAngle = a_angle * 0.5f;

            float sin = (float)Math.Sin(halfAngle);

            return new Quaternion(a_axis.X * sin, a_axis.Y * sin, a_axis.Z * sin, (float)Math.Cos(halfAngle));
        }
        public static Quaternion FromEuler(Vector3 a_euler)
        {
            // Not the most efficent but it works and less hair pulling
            return FromAxisAngle(Vector3.UnitX, a_euler.X) * FromAxisAngle(Vector3.UnitY, a_euler.Y) * FromAxisAngle(Vector3.UnitZ, a_euler.Z);
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

        public static Quaternion operator +(Quaternion a_lhs, Quaternion a_rhs)
        {
            return new Quaternion(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y, a_lhs.Z + a_rhs.Z, a_lhs.W + a_rhs.W);
        }
        public static Quaternion operator -(Quaternion a_lhs, Quaternion a_rhs)
        {
            return new Quaternion(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y, a_lhs.Z - a_rhs.Z, a_lhs.W - a_rhs.W);
        }

        public static Quaternion operator *(Quaternion a_lhs, Quaternion a_rhs)
        {
            return new Quaternion
            (
                a_lhs.W * a_rhs.X + a_lhs.X * a_rhs.W + a_lhs.Y * a_rhs.Z - a_lhs.Z * a_rhs.Y, 
                a_lhs.W * a_rhs.Y + a_lhs.Y * a_rhs.W + a_lhs.Z * a_rhs.X - a_lhs.X * a_rhs.Z, 
                a_lhs.W * a_rhs.Z + a_lhs.Z * a_rhs.W + a_lhs.X * a_rhs.Y - a_lhs.Y * a_rhs.X, 
                a_lhs.W * a_rhs.W - a_lhs.X * a_rhs.X - a_lhs.Y * a_rhs.Y - a_lhs.Z * a_rhs.Z
            );
        }
        public static Quaternion operator *(Quaternion a_lhs, float a_rhs)
        {
            return new Quaternion(a_lhs.X * a_rhs, a_lhs.Y * a_rhs, a_lhs.Z * a_rhs, a_lhs.W * a_rhs);
        }
        public static Vector3 operator *(Quaternion a_lhs, Vector3 a_rhs)
        {
            Vector3 qVec = new Vector3(a_lhs.X, a_lhs.Y, a_lhs.Z);
            Vector3 c = Vector3.Cross(qVec, a_rhs);
            Vector3 cc = Vector3.Cross(qVec, c);

            return a_rhs + ((c * a_lhs.W) + cc) * 2.0f;
        }
        public static Vector4 operator *(Quaternion a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs * a_rhs.XYZ, a_rhs.W);
        }
    
        public static bool operator ==(Quaternion a_lhs, Quaternion a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y && a_lhs.Z == a_rhs.Z && a_lhs.W == a_rhs.W;
        }
        public static bool operator !=(Quaternion a_lhs, Quaternion a_rhs)
        {
            return a_lhs.X != a_rhs.X || a_lhs.Y != a_rhs.Y || a_lhs.Z != a_rhs.Z || a_lhs.W != a_rhs.W;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }
            
            Quaternion vec = (Quaternion)a_obj;

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

        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
            Z /= mag;
            W /= mag;
        }
        public static Quaternion Normalized(Quaternion a_quat)
        {
            float mag = a_quat.Magnitude;

            return new Quaternion(a_quat.X / mag, a_quat.Y / mag, a_quat.Z / mag, a_quat.W / mag);
        }

        public Vector4 ToVector4()
        {
            return new Vector4(X, Y, Z, W);
        }

        public Matrix4 ToMatrix()
        {
            float sqX = X * X;
            float sqY = Y * Y;
            float sqZ = Z * Z;
            float sqW = W * W;

            float inv = 1.0f / (sqX + sqY + sqZ + sqW);

            float v1 = X * Y;
            float v2 = Z * W;
            float v3 = X * Z;
            float v4 = Y * W;
            float v5 = Y * Z;
            float v6 = X * W;

            Matrix4 mat = new Matrix4
            (
                (sqX - sqY - sqZ + sqW) * inv, 2.0f * (v1 - v2) * inv,         2.0f * (v3 + v4) * inv,         0.0f,
                2.0f * (v1 + v2) * inv,        (-sqX + sqY - sqZ + sqW) * inv, 2.0f * (v5 - v6) * inv,         0.0f,
                2.0f * (v3 - v4) * inv,        2.0f * (v5 + v6) * inv,         (-sqX - sqY + sqZ + sqW) * inv, 0.0f,
                0.0f,                          0.0f,                           0.0f,                           1.0f
            );

            return mat;
        }

        public Vector3 ToEuler()
        {
            float np = X * Y + Z * W;

            if (np > 0.5f - float.Epsilon)
            {
                return new Vector3
                (
                    0.0f,
                    2 * Mathf.Atan2(X, W),
                    Mathf.HalfPI
                );
            }
            else if (np < -0.5f + float.Epsilon)
            {
                return new Vector3
                (
                    0.0f, 
                    -2 * Mathf.Atan2(X, W),
                    -Mathf.HalfPI
                );
            }

            float sqX = X * X;
            float sqY = Y * Y;
            float sqZ = Z * Z;

            return new Vector3
            (
                // b
                Mathf.Atan2(2 * X * W - 2 * Y * Z, 1 - 2 * sqX - 2 * sqZ),
                // h
                Mathf.Atan2(2 * Y * W - 2 * X * Z, 1 - 2 * sqY - 2 * sqZ),
                // a
                Mathf.Asin(2 * np)
            );         
        }

        public Vector4 ToAxisAngle()
        {
            if (W >= 1)
            {
                return Vector4.Zero;
            }

            // Use Inverse for div by zero error
            float sInv = 1 / Mathf.Sqrt(1 - W * W);

            return new Vector4
            (
                X * sInv,
                Y * sInv, 
                Z * sInv,
                2 * Mathf.Acos(W)
            );
        }

        public static Quaternion Inverse(Quaternion a_quat)
        {
            float mag = a_quat.MagnitudeSqr;

            return new Quaternion(-a_quat.X / mag, -a_quat.Y / mag, -a_quat.Z / mag, a_quat.W / mag);
        }

        public static Quaternion Slerp(Quaternion a_lhs, Quaternion a_rhs, float a_t)
        {
            // Cant remember off the top of my head so just assuming this works
            // Seems correct as using the inverse to do a local translation but memory is hazy
            // Credit: https://en.wikipedia.org/wiki/Slerp
            Quaternion invQ = Inverse(a_rhs);

            Quaternion q = a_lhs * invQ;
            Quaternion powQ = new Quaternion(Mathf.Pow(q.X, a_t), Mathf.Pow(q.Y, a_t), Mathf.Pow(q.Z, a_t), Mathf.Pow(q.W, a_t));

            return powQ * a_rhs;
        }
        public static Quaternion Lerp(Quaternion a_lhs, Quaternion a_rhs, float a_t)
        {
            return a_lhs * (1.0f - a_t) + a_rhs * a_t;
        }
    }
}