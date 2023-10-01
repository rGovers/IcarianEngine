using System;

namespace IcarianEngine.Maths
{
    public static class Mathf
    {
        public const float PI = (float)Math.PI;
        public const float HalfPI = PI * 0.5f;
        public const float TwoPI = PI * 2.0f;

        public const float DegToRad = PI / 180.0f;
        public const float RadToDeg = 180.0f / PI;
        
        public static float Asin(float a_v)
        {
            return (float)Math.Asin(a_v);
        }
        public static float Acos(float a_v)
        {
            return (float)Math.Acos(a_v);
        }
        public static float Atan2(float a_x, float a_y)
        {
            return (float)Math.Atan2(a_x, a_y);
        }
        public static float Sin(float a_a)
        {
            return (float)Math.Sin(a_a);
        }
        public static float Cos(float a_a)
        {
            return (float)Math.Cos(a_a);
        }
        public static float Tan(float a_a)
        {
            return (float)Math.Tan(a_a);
        }

        public static float Sqrt(float a_a)
        {
            return (float)Math.Sqrt(a_a);
        }
        public static float Pow(float a_a, float a_b)
        {
            return (float)Math.Pow(a_a, a_b);
        }

        public static float Abs(float a_a)
        {
            return (float)Math.Abs(a_a);
        }
        public static float Sign(float a_a)
        {
            if (a_a == 0.0f)
            {
                return 0.0f;
            }
            
            if (a_a < 0.0f)
            {
                return -1.0f;
            }

            return 1.0f;
        }

        public static float Min(float a_a, float a_b)
        {
            if (a_a < a_b)
            {
                return a_a;
            }

            return a_b;
        }
        public static float Max(float a_a, float a_b)
        {
            if (a_a > a_b)
            {
                return a_a;
            }

            return a_b;
        }

        public static float Lerp(float a_a, float a_b, float a_t)
        {
            return a_a + (a_b - a_a) * a_t;
        }
    }
}