using System;

namespace IcarianEngine
{
    public class EditorTooltipAttribute : Attribute
    {
        string m_tooltip;
    
        public string Tooltip 
        {
            get
            {
                return m_tooltip;
            }
        }
    
        public EditorTooltipAttribute(string a_tooltip)
        {
            m_tooltip = a_tooltip;
        }
    }
}
