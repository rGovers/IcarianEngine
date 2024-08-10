using System;

namespace IcarianEngine
{
    public class EditorRangeAttribute : Attribute
    {
        double m_min;
        double m_max;

        /// <summary>
        /// The minimum value for the field
        /// </summary>
        public double Min
        {
            get
            {
                return m_min;
            }
        }
        /// <summary>
        /// The maximum value for the field
        /// </summary>
        public double Max
        {
            get
            {
                return m_max;
            }
        }

        public EditorRangeAttribute(double a_min, double a_max)
        {
            m_min = a_min;
            m_max = a_max;
        }
    }
}