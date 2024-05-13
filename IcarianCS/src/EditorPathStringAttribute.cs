using System;

namespace IcarianEngine
{
    public class EditorPathStringAttribute : Attribute
    {
        string[] m_extensions;

        public string[] Extensions
        {
            get
            {
                return m_extensions;
            }
        }

        public EditorPathStringAttribute(string[] a_extensions)
        {
            m_extensions = a_extensions;
        }
    }
}