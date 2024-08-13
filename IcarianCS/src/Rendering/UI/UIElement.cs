// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineUIElementInterop.h"
#include "EngineUIElementInteropStuctures.h"
#include "InteropBinding.h"

ENGINE_UIELEMENT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class UIElement : IDestroy
    {
        static readonly ConcurrentDictionary<uint, UIElement> s_elementLookup = new ConcurrentDictionary<uint, UIElement>();

        /// <summary>
        /// Delegate for UI events
        /// </summary>
        /// <param name="a_canvas">The <see cref="IcarianEngine.Rendering.UI.Canvas" /> the event is for</param>
        /// <param name="a_element">The <see cref="IcarianEngine.Rendering.UI.UIElement" /> the event is for</param>
        public delegate void UIEvent(Canvas a_canvas, UIElement a_element);

        uint   m_bufferAddr = uint.MaxValue;
        string m_name;

        /// <summary>
        /// Delegate for normal events
        /// </summary>
        public UIEvent OnNormal = null;
        /// <summary>
        /// Delegate for hover events
        /// </summary>
        public UIEvent OnHover = null;
        /// <summary>
        /// Delegate for press events
        /// </summary>
        public UIEvent OnPressed = null;
        /// <summary>
        /// Delegate for release events
        /// </summary>
        public UIEvent OnReleased = null;

        /// <summary>
        /// Whether or not the UIElement is Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
            set
            {
                m_bufferAddr = value;
            }
        }

        /// <summary>
        /// The name of the UIElement
        /// </summary>
        public string Name
        {
            get
            {
                return m_name;
            }
            internal set
            {
                m_name = value;
            }
        }

        /// <summary>
        /// The current state of the UIElement
        /// </summary>
        public ElementState ElementState
        {
            get
            {
                return (ElementState)UIElementInterop.GetElementState(m_bufferAddr);
            }
        }

        /// <summary>
        /// The position of the UIElement in <see cref="IcarianEngine.Rendering.UI.Canvas" /> space
        /// </summary>
        public Vector2 Position
        {
            get
            {
                // Get stack corruption with return value refer to EngineUIElementInterop.h
                IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector2>());

                UIElementInterop.GetPosition(m_bufferAddr, ptr);
                Vector2 val = Marshal.PtrToStructure<Vector2>(ptr);
                Marshal.FreeHGlobal(ptr);

                return val;
            }
            set
            {
                UIElementInterop.SetPosition(m_bufferAddr, value);
            }
        }
        /// <summary>
        /// The size of the UIElement in <see cref="IcarianEngine.Rendering.UI.Canvas" /> space
        /// </summary>
        public Vector2 Size
        {
            get
            {
                // Get stack corruption with return value refer to EngineUIElementInterop.h
                IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector2>());

                UIElementInterop.GetSize(m_bufferAddr, ptr);
                Vector2 val = Marshal.PtrToStructure<Vector2>(ptr);
                Marshal.FreeHGlobal(ptr);

                return val;
            }
            set
            {
                UIElementInterop.SetSize(m_bufferAddr, value);
            }
        }

        /// <summary>
        /// The colour of the UIElement
        /// </summary>
        public Color Color
        {
            get
            {
                return UIElementInterop.GetColor(m_bufferAddr).ToColor();
            }
            set
            {
                UIElementInterop.SetColor(m_bufferAddr, value.ToVector4());
            }
        }

        /// <summary>
        /// The X anchor point for the UIElement
        /// </summary>
        public UIXAnchor XAnchor
        {
            get
            {
                return (UIXAnchor)UIElementInterop.GetElementXAnchor(m_bufferAddr);
            }
            set
            {
                UIElementInterop.SetElementXAnchor(m_bufferAddr, (uint)value);
            }
        }
        /// <summary>
        /// The Y anchor point for the UIElement
        /// </summary>
        public UIYAnchor YAnchor
        {
            get
            {
                return (UIYAnchor)UIElementInterop.GetElementYAnchor(m_bufferAddr);
            }
            set
            {
                UIElementInterop.SetElementYAnchor(m_bufferAddr, (uint)value);
            }
        }
        
        /// <summary>
        /// The children of the UIElement
        /// </summary>
        public IEnumerable<UIElement> Children
        {
            get
            {
                uint[] children = UIElementInterop.GetChildren(m_bufferAddr);

                foreach (uint child in children)
                {
                    if (s_elementLookup.ContainsKey(child))
                    {
                        yield return s_elementLookup[child];
                    }
                }
            }
        }

        public UIElement()
        {
            m_bufferAddr = UIElementInterop.CreateUIElement();

            AddLookup(m_bufferAddr, this);
        }
        protected internal UIElement(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            AddLookup(m_bufferAddr, this);
        }

        /// @cond INTERNAL

        internal void AddLookup(uint a_addr, UIElement a_element)
        {
            s_elementLookup.TryAdd(a_addr, a_element);
        }
        internal void RemoveLookup(uint a_addr)
        {
            s_elementLookup.TryRemove(a_addr, out UIElement _);
        }

        internal static UIElement GetUIElement(uint a_addr)
        {
            if (s_elementLookup.ContainsKey(a_addr))
            {
                return s_elementLookup[a_addr];
            }

            return null;
        }

        /// @endcond

        /// <summary>
        /// Gets the child of the UIElement with name recursively
        /// </summary>
        /// <param name="a_name">The name of the child to get</param>
        /// <returns>The child UIElement. Null on failure</returns>
        public UIElement GetNamedChild(string a_name)
        {
            foreach (UIElement child in Children)
            {
                if (child.Name == a_name)
                {
                    return child;
                }

                UIElement element = child.GetNamedChild(a_name);
                if (element != null)
                {
                    return element;
                }
            }

            return null;
        }
        /// <summary>
        /// Gets the child of the UIElement of type T with name recursively
        /// </summary>
        /// <param name="a_name">The name of the child to get</param>
        /// <returns>The child UIElement. Null on failure</returns>
        public T GetNamedChild<T>(string a_name) where T : UIElement
        {
            return GetNamedChild(a_name) as T;
        }

        /// <summary>
        /// Adds a child to the UIElement
        /// </summary>
        /// <param name="a_child">The child to add</param>
        public void AddChild(UIElement a_child)
        {
            if (a_child == null)
            {
                Logger.IcarianWarning("Null UIElement");

                return;
            }

            UIElementInterop.AddChildElement(BufferAddr, a_child.BufferAddr);
        }
        /// <summary>
        /// Removes a child from the UIElement
        /// </summary>
        /// <param name="a_child">The child to remove</param>
        public void RemoveChild(UIElement a_child)
        {
            if (a_child == null)
            {
                Logger.IcarianWarning("Null UIElement");

                return;
            }

            UIElementInterop.RemoveChildElement(BufferAddr, a_child.BufferAddr);
        }

        static void OnNormalS(uint a_canvasAddr, uint a_elementAddr)
        {
            Canvas canvas = Canvas.GetCanvas(a_canvasAddr);
            UIElement element = GetUIElement(a_elementAddr);

            if (element.OnNormal != null)
            {
                element.OnNormal.Invoke(canvas, element);
            }
        }
        static void OnHoverS(uint a_canvasAddr, uint a_elementAddr)
        {
            Canvas canvas = Canvas.GetCanvas(a_canvasAddr);
            UIElement element = GetUIElement(a_elementAddr);

            if (element.OnHover != null)
            {
                element.OnHover.Invoke(canvas, element);
            }
        }
        static void OnPressedS(uint a_canvasAddr, uint a_elementAddr)
        {
            Canvas canvas = Canvas.GetCanvas(a_canvasAddr);
            UIElement element = GetUIElement(a_elementAddr);

            if (element.OnPressed != null)
            {
                element.OnPressed.Invoke(canvas, element);
            }
        }
        static void OnReleasedS(uint a_canvasAddr, uint a_elementAddr)
        {
            Canvas canvas = Canvas.GetCanvas(a_canvasAddr);
            UIElement element = GetUIElement(a_elementAddr);

            if (element.OnReleased != null)
            {
                element.OnReleased.Invoke(canvas, element);
            }
        }

        /// <summary>
        /// Disposes of the UIElement
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the UIElement is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether or not it is called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    UIElementInterop.DestroyUIElement(m_bufferAddr);

                    RemoveLookup(m_bufferAddr);

                    foreach (UIElement child in Children)
                    {
                        child.Dispose();
                    }
                }
                else
                {
                    Logger.IcarianWarning("UIElement failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Dispose calls on UIElement");
            }
        }
        ~UIElement()
        {
            Dispose(false);
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