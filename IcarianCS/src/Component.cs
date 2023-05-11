using IcarianEngine.Definitions;
using System;

namespace IcarianEngine
{
    public class Component 
    {
        ComponentDef m_def = null;

        GameObject   m_object;

        public ComponentDef Def
        {
            get
            {
                return m_def;
            }
        }

        public GameObject Object
        {
            get
            {
                return m_object;
            }
            internal set
            {
                m_object = value;
            }
        }

        public Transform Transform
        {
            get
            {
                return m_object.Transform;
            }
        }

        internal Component() { }

        public virtual void Init() { }

        internal static Component FromDef(ComponentDef a_def) 
        {
            Component comp = Activator.CreateInstance(a_def.ComponentType) as Component;

            if (comp != null)
            {
                comp.m_def = a_def;
            }

            return comp;
        }
        internal static T FromDef<T>(ComponentDef a_def) where T : Component
        {
            return FromDef(a_def) as T;
        }
    }
}