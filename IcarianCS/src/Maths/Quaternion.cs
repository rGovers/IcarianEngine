// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;
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
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
            [MethodImpl(MethodImplOptions.AggressiveInlining)]
            get
            {
                return Mathf.Sqrt(MagnitudeSqr);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Quaternion(float a_x, float a_y, float a_z, float a_w)
        {
            X = a_x;
            Y = a_y;
            Z = a_z;
            W = a_w;
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Quaternion(Quaternion a_other)
        {
            X = a_other.X;
            Y = a_other.Y;
            Z = a_other.Z;
            W = a_other.W;
        }

        /// <summary>
        /// Creates a Quaternion from direction vectors
        /// </summary>
        /// <param name="a_right">Right vector</param>
        /// <param name="a_up">Up vector</param>
        /// <param name="a_forward">Forward vector</param>
        /// <returns>The Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion FromDirectionVectors(Vector3 a_right, Vector3 a_up, Vector3 a_forward)
        {
            // Credit to glm quaternion
            // 3 direction vectors are basically a mat3
            // Someone is gonna throw semantics
            float xSqrInv = a_right.X - a_up.Y - a_forward.Z;
            float ySqrInv = a_up.Y - a_right.X - a_forward.Z;
            float zSqrInv = a_forward.Z - a_right.X - a_up.Y;
            float wSqrInv = a_right.X + a_up.Y + a_forward.Z;

            uint maxIndex = 0;
            float maxValSqr = wSqrInv;
            
            if (xSqrInv > maxValSqr)
            {
                maxValSqr = xSqrInv;
                maxIndex = 1;
            }
            if (ySqrInv > maxValSqr)
            {
                maxValSqr = ySqrInv;
                maxIndex = 2;
            }
            if (zSqrInv > maxValSqr)
            {
                maxValSqr = zSqrInv;
                maxIndex = 3;
            }

            float val = Mathf.Sqrt(maxValSqr + 1.0f) * 0.5f;
            float mul = 0.25f / val;

            switch (maxIndex)
            {
            case 0:
            {
                return new Quaternion
                (
                    (a_up.Z - a_forward.Y) * mul,
                    (a_forward.X - a_right.Z) * mul,
                    (a_right.Y - a_up.X) * mul,
                    val
                );
            }
            case 1:
            {
                return new Quaternion
                (
                    val,
                    (a_right.Y + a_up.X) * mul,
                    (a_forward.X + a_right.Z) * mul,
                    (a_up.Z - a_forward.Y) * mul
                );
            }
            case 2:
            {
                return new Quaternion
                (
                    (a_right.Y + a_up.X) * mul,
                    val,
                    (a_up.Z + a_forward.Y) * mul,
                    (a_forward.X - a_right.Z) * mul
                );
            }
            case 3:
            {
                return new Quaternion
                (
                    (a_forward.X + a_right.Z) * mul,
                    (a_up.Z + a_forward.Y) * mul,
                    val,
                    (a_right.Y - a_up.X) * mul
                );
            }
            }

            Logger.IcarianError("Invalid direction vector quaternion");

            return Quaternion.Identity;
        }
        /// <summary>
        /// Creates a Quaternion from a rotation matrix
        /// </summary>
        /// <param name="a_mat">The rotation matrix to use</param>
        /// <returns>The Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion FromMatrix(Matrix4 a_mat)
        {
            return FromDirectionVectors(a_mat[0].XYZ, a_mat[1].XYZ, a_mat[2].XYZ);
        }
        /// <summary>
        /// Creates a Quaternion from an axis and angle
        /// </summary>
        /// <param name="a_axis">The axis to use</param>
        /// <param name="a_angle">The angle to use</param>
        /// <returns>The Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion FromAxisAngle(Vector3 a_axis, float a_angle)
        {
            float halfAngle = a_angle * 0.5f;

            float sin = Mathf.Sin(halfAngle);

            return new Quaternion(a_axis.X * sin, a_axis.Y * sin, a_axis.Z * sin, Mathf.Cos(halfAngle));
        }
        /// <summary>
        /// Creates a Quaternion from euler angles
        /// </summary>
        /// <param name="a_euler">The euler angles to use</param>
        /// <returns>The Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion FromEuler(Vector3 a_euler)
        {
            // Not the most efficent but it works and less hair pulling
            return FromAxisAngle(Vector3.UnitX, a_euler.X) * FromAxisAngle(Vector3.UnitY, a_euler.Y) * FromAxisAngle(Vector3.UnitZ, a_euler.Z);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion operator +(Quaternion a_lhs, Quaternion a_rhs)
        {
            return new Quaternion(a_lhs.X + a_rhs.X, a_lhs.Y + a_rhs.Y, a_lhs.Z + a_rhs.Z, a_lhs.W + a_rhs.W);
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion operator -(Quaternion a_lhs, Quaternion a_rhs)
        {
            return new Quaternion(a_lhs.X - a_rhs.X, a_lhs.Y - a_rhs.Y, a_lhs.Z - a_rhs.Z, a_lhs.W - a_rhs.W);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion operator *(Quaternion a_lhs, float a_rhs)
        {
            return new Quaternion(a_lhs.X * a_rhs, a_lhs.Y * a_rhs, a_lhs.Z * a_rhs, a_lhs.W * a_rhs);
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion operator /(Quaternion a_lhs, float a_rhs)
        {
            return new Quaternion(a_lhs.X / a_rhs, a_lhs.Y / a_rhs, a_lhs.Z / a_rhs, a_lhs.W / a_rhs);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector3 operator *(Quaternion a_lhs, Vector3 a_rhs)
        {
            Vector3 qVec = new Vector3(a_lhs.X, a_lhs.Y, a_lhs.Z);
            Vector3 c = Vector3.Cross(qVec, a_rhs);
            Vector3 cc = Vector3.Cross(qVec, c);

            return a_rhs + ((c * a_lhs.W) + cc) * 2.0f;
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Vector4 operator *(Quaternion a_lhs, Vector4 a_rhs)
        {
            return new Vector4(a_lhs * a_rhs.XYZ, a_rhs.W);
        }
        
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion operator -(Quaternion a_quat)
        {
            return new Quaternion(-a_quat.X, -a_quat.Y, -a_quat.Z, -a_quat.W);
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static bool operator ==(Quaternion a_lhs, Quaternion a_rhs)
        {
            return a_lhs.X == a_rhs.X && a_lhs.Y == a_rhs.Y && a_lhs.Z == a_rhs.Z && a_lhs.W == a_rhs.W;
        }
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
        /// Normalizes the Quaternion
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public void Normalize()
        {
            float mag = Magnitude;

            X /= mag;
            Y /= mag;
            Z /= mag;
            W /= mag;
        }
        /// <summary>
        /// Returns a normalized Quaternion
        /// </summary>
        /// <param name="a_quat">The Quaternion to normalize</param>
        /// <returns>The normalized Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion Normalized(Quaternion a_quat)
        {
            float mag = a_quat.Magnitude;

            return new Quaternion(a_quat.X / mag, a_quat.Y / mag, a_quat.Z / mag, a_quat.W / mag);
        }

        /// <summary>
        /// Converts the quaternion to a Vector4
        /// </summary>
        /// <returns>The Vector4</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Vector4 ToVector4()
        {
            return new Vector4(X, Y, Z, W);
        }

        /// <summary>
        /// Converts the Quaternion to a rotation matrix
        /// </summary>
        /// <returns>The rotation matrix</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public Matrix4 ToMatrix()
        {
            float sqX = X * X;
            float sqY = Y * Y;
            float sqZ = Z * Z;

            float qXZ = X * Z;
            float qXY = X * Y;
            float qYZ = Y * Z;
            float qWX = W * X;
            float qWY = W * Y;
            float qWZ = W * Z;

            return new Matrix4
            (
                1.0f - 2.0f * (sqY + sqZ), 2.0f * (qXY + qWZ),        2.0f * (qXZ - qWY),        0.0f,
                2.0f * (qXY - qWZ),        1.0f - 2.0f * (sqX + sqZ), 2.0f * (qYZ + qWX),        0.0f,
                2.0f * (qXZ + qWY),        2.0f * (qYZ - qWX),        1.0f - 2.0f * (sqX + sqY), 0.0f,
                0.0f,                      0.0f,                      0.0f,                      1.0f
            );
        }

        /// <summary>
        /// Converts the Quaternion to euler angles
        /// </summary>
        /// <returns>The euler angles</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
        /// Converts the Quaternion to an axis angle
        /// </summary>
        /// XYZ = Axis
        /// W = Angle
        /// <returns>The axis angle</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
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
        /// Inverts the Quaternion
        /// </summary>
        /// <param name="a_quat">The Quaternion to invert</param>
        /// <returns>The inverted Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion Inverse(Quaternion a_quat)
        {
            float mag = a_quat.MagnitudeSqr;

            return new Quaternion(-a_quat.X / mag, -a_quat.Y / mag, -a_quat.Z / mag, a_quat.W / mag);
        }

        /// <summary>
        /// Gets the dot product of two Quaterions
        /// </summary>
        /// <param name="a_lhs">Left hand side of the dot product</param>
        /// <param name="a_rhs">Right hand side of the dot product</param>
        /// <returns>Dot product of the two Quaterions</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Dot(Quaternion a_lhs, Quaternion a_rhs)
        {
            return a_lhs.X * a_rhs.X + a_lhs.Y * a_rhs.Y + a_lhs.Z * a_rhs.Z + a_lhs.W * a_rhs.W;
        }

        /// <summary>
        /// Spherical linear interpolation between two Quaternions
        /// </summary>
        /// <param name="a_lhs">From Quaternion</param>
        /// <param name="a_rhs">To Quaternion</param>
        /// <param name="a_t">The interpolation value</param>
        /// <returns>The interpolated Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion Slerp(Quaternion a_lhs, Quaternion a_rhs, float a_t)
        {
            Quaternion z = a_rhs;

            float dot = Dot(a_lhs, a_rhs);
            if (dot < 0.0f)
            {
                z = -a_rhs;
                dot = -dot;
            }

            // Dont mind me screaming in the corner because 1 - float.Epsilon == 1, fuck floating point numbers
            // NaN are you okay?~ Are you okay, NaN?~
            // You've been hit by~, You've been struck by~. Float!~ Awo~
            if (dot > 0.99f)
            {
                return Quaternion.Lerp(a_lhs, z, a_t);
            }

            float a = Mathf.Acos(dot);
            return (a_lhs * Mathf.Sin((1 - a_t) * a) + z * Mathf.Sin(a_t * a)) / Mathf.Sin(a);
        }
        /// <summary>
        /// Linear interpolation between two Quaternions
        /// </summary>
        /// <param name="a_lhs">From Quaternion</param>
        /// <param name="a_rhs">To Quaternion</param>
        /// <param name="a_t">The interpolation value</param>
        /// <returns>The interpolated Quaternion</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static Quaternion Lerp(Quaternion a_lhs, Quaternion a_rhs, float a_t)
        {
            return a_lhs * (1.0f - a_t) + a_rhs * a_t;
        }

        /// @cond SWIZZLE

        VEC_SWIZZLE_QUAT_FULL_VEC2
        VEC_SWIZZLE_QUAT_FULL_VEC3
        VEC_SWIZZLE_QUAT_FULL_VEC4

        /// @endcond
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