using System;
using System.Runtime.InteropServices;
using System.Xml;

#include "Swizzle.h"

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

        public static XmlElement ToXml(this Quaternion a_quat, XmlDocument a_doc, string a_name)
        {
            if (a_quat == Quaternion.Identity)
            {
                return null;
            }

            XmlElement element = a_doc.CreateElement(a_name);

            if (a_quat.X != Quaternion.Identity.X)
            {
                XmlElement x = a_doc.CreateElement("X");
                x.InnerText = a_quat.X.ToString();
                element.AppendChild(x);
            }
            if (a_quat.Y != Quaternion.Identity.Y)
            {
                XmlElement y = a_doc.CreateElement("Y");
                y.InnerText = a_quat.Y.ToString();
                element.AppendChild(y);
            }
            if (a_quat.Z != Quaternion.Identity.Z)
            {
                XmlElement z = a_doc.CreateElement("Z");
                z.InnerText = a_quat.Z.ToString();
                element.AppendChild(z);
            }
            if (a_quat.W != Quaternion.Identity.W)
            {
                XmlElement w = a_doc.CreateElement("W");
                w.InnerText = a_quat.W.ToString();
                element.AppendChild(w);
            }

            return element;
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Quaternion
    {
        /// <summary>
        /// The X component of the quaternion
        /// </summary>
        [FieldOffset(0)]
        public float X;
        /// <summary>
        /// The Y component of the quaternion
        /// </summary>
        [FieldOffset(4)]
        public float Y;
        /// <summary>
        /// The Z component of the quaternion
        /// </summary>
        [FieldOffset(8)]
        public float Z;
        /// <summary>
        /// The W component of the quaternion
        /// </summary>
        [FieldOffset(12)]
        public float W;

        /// <summary>
        /// Identity quaternion
        /// </summary>
        public static readonly Quaternion Identity = new Quaternion(0.0f, 0.0f, 0.0f, 1.0f); 

        /// <summary>
        /// The squared magnitude of the quaternion
        /// </summary>
        public float MagnitudeSqr
        {
            get
            {
                return X * X + Y * Y + Z * Z + W * W;
            }
        }
        /// <summary>
        /// The magnitude of the quaternion
        /// </summary>
        public float Magnitude
        {
            get
            {
                return (float)Math.Sqrt(MagnitudeSqr);
            }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="a_x">X component</param>
        /// <param name="a_y">Y component</param>
        /// <param name="a_z">Z component</param>
        /// <param name="a_w">W component</param>
        public Quaternion(float a_x, float a_y, float a_z, float a_w)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
            W = a_w;
        }
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="a_vec">Vector to copy</param>
        public Quaternion(Quaternion a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }

        /// <summary>
        /// Creates a quaternion from direction vectors
        /// </summary>
        /// <param name="a_right">Right vector</param>
        /// <param name="a_up">Up vector</param>
        /// <param name="a_forward">Forward vector</param>
        /// <returns>The Quaternion</returns>
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
        /// <summary>
        /// Creates a quaternion from a rotation matrix
        /// </summary>
        /// <param name="a_mat">The rotation matrix</param>
        /// <returns>The Quaternion</returns>
        public static Quaternion FromMatrix(Matrix4 a_mat)
        {
            return FromDirectionVectors(a_mat[0].XYZ, a_mat[1].XYZ, a_mat[2].XYZ);
        }
        /// <summary>
        /// Creates a quaternion from an axis and angle
        /// </summary>
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
    
        public static Quaternion operator -(Quaternion a_quat)
        {
            return new Quaternion(-a_quat.X, -a_quat.Y, -a_quat.Z, -a_quat.W);
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

        /// <summary>
        /// Normalizes the quaternion
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
        /// Returns a normalized quaternion
        /// </summary>
        /// <param name="a_quat">The quaternion to normalize</param>
        /// <returns>The normalized quaternion</returns>
        public static Quaternion Normalized(Quaternion a_quat)
        {
            float mag = a_quat.Magnitude;

            return new Quaternion(a_quat.X / mag, a_quat.Y / mag, a_quat.Z / mag, a_quat.W / mag);
        }

        /// <summary>
        /// Converts the quaternion to a Vector4
        /// </summary>
        /// <returns>The Vector4</returns>
        public Vector4 ToVector4()
        {
            return new Vector4(X, Y, Z, W);
        }

        /// <summary>
        /// Converts the quaternion to a rotation matrix
        /// </summary>
        /// <returns>The rotation matrix</returns>
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

        /// <summary>
        /// Converts the quaternion to euler angles
        /// </summary>
        /// <returns>The euler angles</returns>
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
        
        /// <summary>
        /// Converts the quaternion to an axis angle
        /// </summary>
        /// XYZ = Axis
        /// W = Angle
        /// <returns>The axis angle</returns>
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

        /// <summary>
        /// Inverts the quaternion
        /// </summary>
        /// <param name="a_quat">The quaternion to invert</param>
        /// <returns>The inverted quaternion</returns>
        public static Quaternion Inverse(Quaternion a_quat)
        {
            float mag = a_quat.MagnitudeSqr;

            return new Quaternion(-a_quat.X / mag, -a_quat.Y / mag, -a_quat.Z / mag, a_quat.W / mag);
        }

        /// <summary>
        /// Spherical linear interpolation between two quaternions
        /// </summary>
        /// <param name="a_lhs">From quaternion</param>
        /// <param name="a_rhs">To quaternion</param>
        /// <param name="a_t">The interpolation value</param>
        /// <returns>The interpolated quaternion</returns>
        public static Quaternion Slerp(Quaternion a_lhs, Quaternion a_rhs, float a_t)
        {
            // TODO: Need to fix this
            // Cant remember off the top of my head so just assuming this works
            // Seems correct as using the inverse to do a local translation but memory is hazy
            // Credit: https://en.wikipedia.org/wiki/Slerp
            Quaternion invQ = Inverse(a_rhs);

            Quaternion q = a_lhs * invQ;
            Vector4 sign = new Vector4(Math.Sign(q.X), Math.Sign(q.Y), Math.Sign(q.Z), Math.Sign(q.W));
            Vector4 abs = new Vector4(Math.Abs(q.X), Math.Abs(q.Y), Math.Abs(q.Z), Math.Abs(q.W));
            Quaternion powQ = new Quaternion(Mathf.Pow(abs.X, a_t) * sign.X, Mathf.Pow(abs.Y, a_t) * sign.Y, Mathf.Pow(abs.Z, a_t) * sign.Z, Mathf.Pow(abs.W, a_t) * sign.W);

            return powQ * a_rhs;
        }
        /// <summary>
        /// Linear interpolation between two quaternions
        /// </summary>
        /// <param name="a_lhs">From quaternion</param>
        /// <param name="a_rhs">To quaternion</param>
        /// <param name="a_t">The interpolation value</param>
        public static Quaternion Lerp(Quaternion a_lhs, Quaternion a_rhs, float a_t)
        {
            return a_lhs * (1.0f - a_t) + a_rhs * a_t;
        }

        VEC_SWIZZLE_QUAT_FULL_VEC2
        VEC_SWIZZLE_QUAT_FULL_VEC3
        VEC_SWIZZLE_QUAT_FULL_VEC4
    }
}