using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Maths
{
    public static class Mathf
    {
        /// <summary>
        /// The value of PI
        /// </summary>
        public const float PI = (float)Math.PI;
        /// <summary>
        /// A quarter the value of PI
        /// </summary>
        public const float QuarterPI = PI * 0.25f;
        /// <summary>
        /// Half the value of PI
        /// </summary>
        public const float HalfPI = PI * 0.5f;
        /// <summary>
        /// Double the value of PI
        /// </summary>
        public const float TwoPI = PI * 2.0f;

        /// <summary>
        /// Constant value to convert degrees to radians
        /// </summary>
        public const float DegToRad = PI / 180.0f;
        /// <summary>
        /// Constant value to convert radians to degrees
        /// </summary>
        public const float RadToDeg = 180.0f / PI;
        
        /// <summary>
        /// Gets the arcsine of the value
        /// </summary>
        /// <param name="a_v">The value to get the arcsine of</param>
        /// <returns>The arcsine of the value in radians</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Asin(float a_v)
        {
            return (float)Math.Asin(a_v);
        }
        /// <summary>
        /// Gets the arccosine of the value
        /// </summary>
        /// <param name="a_v">The value to get te arccosine of</param>
        /// <returns>The arccosine of the value in radians</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Acos(float a_v)
        {
            return (float)Math.Acos(a_v);
        }
        /// <summary>
        /// Gets the arctangent of 2 input values
        /// </summary>
        /// <param name="a_x">The first value to get the arctangent of</param>
        /// <param name="a_y">The second value to get the arctangent of</param>
        /// <returns>The arctangent of the values in radians</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Atan2(float a_x, float a_y)
        {
            return (float)Math.Atan2(a_x, a_y);
        }

        /// <summary>
        /// Gets the sine of the value
        /// </summary>
        /// <param name="a_a">The value to get the sine of</param>
        /// <returns>The sine of the value in radians</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Sin(float a_a)
        {
            return (float)Math.Sin(a_a);
        }
        /// <summary>
        /// Gets the cosine of the value
        /// </summary>
        /// <param name="a_a">The value to get the cosine of</param>
        /// <returns>The cosine of the value in radians</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Cos(float a_a)
        {
            return (float)Math.Cos(a_a);
        }
        /// <summary>
        /// Gets the tangent of the value
        /// </summary>
        /// <param name="a_a">The value to get the tangent of</param>
        /// <returns>The tangent of the value in radians</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Tan(float a_a)
        {
            return (float)Math.Tan(a_a);
        }

        /// <summary>
        /// Gets the square root of the value
        /// <summary>
        /// <param name="a_a">The value to get the square root of</param>
        /// <returns>The square root of the value</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Sqrt(float a_a)
        {
            return (float)Math.Sqrt(a_a);
        }
        /// <summary>
        /// Raises a value by a power
        /// </summary>
        /// <param name="a_a">The value to raise</param>
        /// <param name="a_b">The power to raise it by</param>
        /// <returns>The value raised by the power</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Pow(float a_a, float a_b)
        {
            return (float)Math.Pow(a_a, a_b);
        }

        /// <summary>
        /// Gets the absolute value of the input
        /// </summary>
        /// <param name="a_a">The value to get the absolute of</param>
        /// <returns>The absolute of the input</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Abs(float a_a)
        {
            return a_a * Sign(a_a);
        }
        /// <summary>
        /// Gets the sign value of the input
        /// </summary>
        /// <param name="a_a">The value to get the Sign of</param>
        /// <returns>The sign of the input value as 1 or -1</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int Sign(float a_a)
        {
            // Have to do C# specific shit rather then just bit mask
            // System one is a piece of shit
            unchecked
            {
                return (a_a.GetHashCode() >> 31) * 2 + 1;
            }
        }
        /// <summary>
        /// Gets the sign value of the input
        /// </summary>
        /// <param name="a_a">The value to get the Sign of</param>
        /// <returns>The sign of the input value as 1 or -1</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static int Sign(int a_a)
        {
            unchecked
            {
                return (a_a >> 31) * 2 + 1;
            }
        }

        /// <summary>
        /// Gets the minimum of 2 values
        /// </summary>
        /// <param name="a_a">The first value to get the minimum value of</param>
        /// <param name="a_b">The second value to get the minimum value of</param>
        /// <returns>The mimimum of the values</returns>
        public static float Min(float a_a, float a_b)
        {
            unchecked
            {
                // I suspect the JIT is actually good as I cannot find a difference between this one and the System.Math.Min upto several million interations
                // Gonna leave as it already works
                int v = -((a_b - a_a).GetHashCode() >> 31);
                int invV = 1 - v;

                return a_a * invV + a_b * v;
            }
            
        }
        /// <summary>
        /// Gets the maximum of 2 values
        /// </summary>
        /// <param name="a_a">The first value to get the maximum of</param>
        /// <param name="a_b">The second value to get the maximum of</param>
        /// <returns>The maximum of the values</returns>
        public static float Max(float a_a, float a_b)
        {
            unchecked
            {
                int v = -((a_b - a_a).GetHashCode() >> 31);
                int invV = 1 - v;

                return a_a * v + a_b * invV;
            }
        }
        /// <summary>
        /// Clamps a value in a range
        /// </summary>
        /// <param name="a_a">The value to clamp</param>
        /// <param name="a_min">The minimum value for the value</param>
        /// <param name="a_max">The maximum value for the value</param>
        /// <returns>The value clamped to the range</returns>
        public static float Clamp(float a_a, float a_min, float a_max)
        {
            return Mathf.Min(a_max, Mathf.Max(a_a, a_min));
        }

        /// <summary>
        /// Gets the minimum of 2 values
        /// </summary>
        /// <param name="a_a">The first value to get the minimum value of</param>
        /// <param name="a_b">The second value to get the minimum value of</param>
        /// <returns>The mimimum of the values</returns>
        public static int Min(int a_a, int a_b)
        {
            unchecked
            {
                int v = -((a_b - a_a) >> 31);
                int invV = 1 - v;

                return a_a * invV + a_b * v;
            }   
        }
        /// <summary>
        /// Gets the minimum of 2 values
        /// </summary>
        /// <param name="a_a">The first value to get the minimum value of</param>
        /// <param name="a_b">The second value to get the minimum value of</param>
        /// <returns>The mimimum of the values</returns>
        public static uint Min(uint a_a, uint a_b)
        {
            // Unsigned so cannot use sign bit to get min
            if (a_a < a_b)
            {
                return a_a;
            }

            return a_b;
        }
        /// <summary>
        /// Gets the maximum of 2 values
        /// </summary>
        /// <param name="a_a">The first value to get the maximum of</param>
        /// <param name="a_b">The second value to get the maximum of</param>
        /// <returns>The maximum of the values</returns>
        public static int Max(int a_a, int a_b)
        {
            unchecked
            {
                int v = -((a_b - a_a) >> 31);
                int invV = 1 - v;

                return a_a * v + a_b * invV;
            }
        }
        /// <summary>
        /// Gets the maximum of 2 values
        /// </summary>
        /// <param name="a_a">The first value to get the maximum of</param>
        /// <param name="a_b">The second value to get the maximum of</param>
        /// <returns>The maximum of the values</returns>
        public static uint Max(uint a_a, uint a_b)
        {
            // Unsigned so cannot use sign bit to get max
            if (a_a > a_b)
            {
                return a_a;
            }

            return a_b;
        }
        /// <summary>
        /// Clamps a value in a range
        /// </summary>
        /// <param name="a_a">The value to clamp</param>
        /// <param name="a_min">The minimum value for the value</param>
        /// <param name="a_max">The maximum value for the value</param>
        /// <returns>The value clamped to the range</returns>
        public static int Clamp(int a_a, int a_min, int a_max)
        {
            return Mathf.Min(a_max, Mathf.Max(a_a, a_min));
        }

        /// <summary>
        /// Floors the input value
        /// </summary>
        /// <param name="a_a">The value to floor</param>
        /// <returns>The floored value</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Floor(float a_a)
        {
            return (float)Math.Floor(a_a);
        }
        /// <summary>
        /// Gets the ceiling of the input value
        /// </summary>
        /// <param name="a_a">The value to ceil</param>
        /// <returns>The ceiling of the value</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Ceil(float a_a)
        {
            return (float)Math.Ceiling(a_a);
        }
        /// <summary>
        /// Rounds the input value
        /// </summary>
        /// <param name="a_a">The value to round</param>
        /// <returns>The rounded value</returns>
        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        public static float Round(float a_a)
        {
            return (float)Math.Round(a_a);
        }

        /// <summary>
        /// Interoplates between 2 values
        /// </summary>
        /// <param name="a_a">The point to interoplate from</param>
        /// <param name="a_b">The point to interoplate to</param>
        /// <param name="a_t">The value to interoplate by</param>
        /// <returns>The interpolated value</returns>
        public static float Lerp(float a_a, float a_b, float a_t)
        {
            return a_a + (a_b - a_a) * a_t;
        }
    }
}