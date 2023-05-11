using System.Runtime.InteropServices;
using System.Xml;

namespace IcarianEngine.Maths
{
    public static class ColorExtensions
    {
        public static Color ToColor(this XmlElement a_element)
        {
            return ToColor(a_element, Color.White);
        }
        public static Color ToColor(this XmlElement a_element, Color a_default)
        {
            Color color = a_default;

            foreach (XmlElement element in a_element)
            {
                switch (element.Name)
                {
                case "R":
                {
                    color.R = byte.Parse(element.InnerText);

                    break;
                }
                case "G":
                {
                    color.G = byte.Parse(element.InnerText);

                    break;
                }
                case "B":
                {
                    color.B = byte.Parse(element.InnerText);

                    break;
                }
                case "A":
                {
                    color.A = byte.Parse(element.InnerText);

                    break;
                }
                }
            }

            return color;
        }
    }

    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Color
    {
        [FieldOffset(0)]
        public byte R;
        [FieldOffset(1)]
        public byte G;
        [FieldOffset(2)]
        public byte B;
        [FieldOffset(3)]
        public byte A;

        public static readonly Color White = Color.FromColorCode(0xffffffff);
        public static readonly Color Black = Color.FromColorCode(0x000000ff);

        public static readonly Color Red = Color.FromColorCode(0xff0000ff);
        public static readonly Color Green = Color.FromColorCode(0x00ff00ff);
        public static readonly Color Blue = Color.FromColorCode(0x0000ffff); 

        public static readonly Color Yellow = Color.FromColorCode(0xffff00ff);
        public static readonly Color Magenta = Color.FromColorCode(0xff00ffff);
        public static readonly Color Aqua = Color.FromColorCode(0x00ffffff);

        public Color(byte a_val)
        {
            R = a_val;
            G = a_val;
            B = a_val;
            A = a_val;
        }
        public Color(byte a_r, byte a_g, byte a_b)
        {
            R = a_r;
            G = a_g;
            B = a_b;
            A = 255;
        }
        public Color(byte a_r, byte a_g, byte a_b, byte a_a)
        {
            R = a_r;
            G = a_g;
            B = a_b;
            A = a_a;
        }

        public static bool operator ==(Color a_lhs, Color a_rhs)
        {
            return a_lhs.R == a_rhs.R && a_lhs.G == a_rhs.G && a_lhs.B == a_rhs.B && a_lhs.A == a_rhs.A;
        }
        public static bool operator !=(Color a_lhs, Color a_rhs)
        {
            return a_lhs.R != a_rhs.R || a_lhs.G != a_rhs.G || a_lhs.B != a_rhs.B || a_lhs.A != a_rhs.A;
        }

        public override int GetHashCode()
        {
            unchecked
            {
                // Lazy so just prime hash till it bites me
                int hash = 73;
                hash = hash * 79 + R.GetHashCode();
                hash = hash * 79 + G.GetHashCode();
                hash = hash * 79 + B.GetHashCode();
                hash = hash * 79 + A.GetHashCode();
                return hash;
            }
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null || !this.GetType().Equals(a_obj.GetType()))
            {
                return false;
            }

            Color color = (Color)a_obj;

            return R == color.R && G == color.G && B == color.B && A == color.A;
        }

        public override string ToString()
        {
            return $"({R}, {G}, {B}, {A})";
        }

        public static Color FromColorCode(uint a_code)
        {
            return new Color((byte)(a_code >> 24), (byte)(a_code >> 16), (byte)(a_code >> 8), (byte)(a_code >> 0));
        }

        public Vector4 ToVector4()
        {
            return new Vector4(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
        }
    }
}