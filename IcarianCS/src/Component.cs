using IcarianEngine.Definitions;
using System;

namespace IcarianEngine
{
    public class Component 
    {
        ComponentDef m_def = null;

        GameObject   m_object;

        /// <summary>
        /// The <see cref="IcarianEngine.Definitions.ComponentDef" /> used to create the Component
        /// </summary>
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

        /// <summary>
        /// The <see cref="IcarianEngine.GameObject" /> the Component is attached to
        /// </summary>
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

        /// <summary>
        /// The <see cref="IcarianEngine.Transform" /> of the <see cref="IcarianEngine.GameObject" /> the Component is attached to
        /// </summary>
        public Transform Transform
        {
            get
            {
                return m_object.Transform;
            }
        }

        /// <summary>
        /// Called when the Component is created
        /// </summary>
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
        
        /// <summary>
        /// Adds a Component of type T to the <see cref="IcarianEngine.GameObject" /> the Component is attached to
        /// </summary>
        /// <returns>The added Component</returns>
        public T AddComponent<T>() where T : Component
        {
            return m_object.AddComponent<T>();
        }
        /// <summary>
        /// Adds a Component of type T from a <see cref="IcarianEngine.Definitions.ComponentDef" /> to the <see cref="IcarianEngine.GameObject" /> the Component is attached to
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to add to the <see cref="IcarianEngine.GameObject" /></param>
        /// <returns>The added Component</returns>
        public T AddComponent<T>(ComponentDef a_def) where T : Component
        {
            return m_object.AddComponent<T>(a_def);
        }
        /// <summary>
        /// Adds a Component from a <see cref="IcarianEngine.Definitions.ComponentDef" /> to the <see cref="IcarianEngine.GameObject" /> the Component is attached to
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to add to the <see cref="IcarianEngine.GameObject" /></param>
        /// <returns>The added Component</returns>
        public Component AddComponent(ComponentDef a_def)
        {
            return m_object.AddComponent(a_def);
        }
        /// <summary>
        /// Gets a Component of type T from the <see cref="IcarianEngine.GameObject" /> the Component is attached to
        /// </summary>
        /// <returns>The Component of type T. Null on failure</returns>
        public T GetComponent<T>() where T : Component
        {
            return m_object.GetComponent<T>();
        }
        /// <summary>
        /// Gets a Component of type T from the <see cref="IcarianEngine.GameObject" /> the Component is attached to that matches the <see cref="IcarianEngine.Definitions.ComponentDef" />
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to get from the <see cref="IcarianEngine.GameObject" /></param>
        /// <returns>The Component from the <see cref="IcarianEngine.Definitions.ComponentDef" />. Null on failure</returns>
        public T GetComponent<T>(ComponentDef a_def) where T : Component
        {
            return m_object.GetComponent<T>(a_def);
        }
        /// <summary>
        /// Gets a Component from the <see cref="IcarianEngine.GameObject" /> the Component is attached to that matches the <see cref="IcarianEngine.Definitions.ComponentDef" />
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.ComponentDef" /> to get from the <see cref="IcarianEngine.GameObject" /></param>
        /// <returns>The Component from the <see cref="IcarianEngine.Definitions.ComponentDef" />. Null on failure</returns>
        public Component GetComponent(ComponentDef a_def)
        {
            return m_object.GetComponent(a_def);
        }
    }
}