// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

            if (!(comp is null))
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

        // Had a nasty bug in a game and while techincally a game bug decided that nipping it in the bud so it cannot happen again was better
        // So now have comparison operators that will check the state of Components and GameObjects and will just equal null if they are invalidated
        public override bool Equals(object a_obj)
        {
            if (base.Equals(a_obj))
            {
                return true;
            }

            if (a_obj == null)
            {
                if (m_object == null)
                {
                    return true;
                }

                if (this is IDestroy dest && dest.IsDisposed)
                {
                    return true;
                }
            }

            return false;
        }

        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public static bool operator ==(Component a_lhs, Component a_rhs)
        {
            if (a_lhs is null)
            {
                return a_rhs is null;
            }

            return a_lhs.Equals(a_rhs);
        }
        public static bool operator !=(Component a_lhs, Component a_rhs)
        {
            return !(a_lhs == a_rhs);
        }
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