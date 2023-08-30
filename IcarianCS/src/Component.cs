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
            internal set
            {
                m_def = value;
            }
        }

        public GameObject GameObject
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

        public T GetComponent<T>() where T : Component
        {
            return m_object.GetComponent<T>();
        }
        public T GetComponent<T>(ComponentDef a_def) where T : Component
        {
            return m_object.GetComponent<T>(a_def);
        }
        public Component GetComponent(ComponentDef a_def)
        {
            return m_object.GetComponent(a_def);
        }
    }
}