using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering.UI
{
    public enum ElementState : uint
    {
        Normal = 0,
        Hovered = 1,
        Pressed = 2,
        Released = 3
    };

    public abstract class UIElement : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddChildElement(uint a_addr, uint a_childAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RemoveChildElement(uint a_addr, uint a_childAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint[] GetChildren(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        // extern static Vector2 GetPosition(uint a_addr);
        // Getting stack corruption on Windows 10 using extern static Vector2 GetPosition(uint a_addr);
        // Fix is to use IntPtr instead of Vector2
        extern static void GetPosition(uint a_addr, IntPtr a_ptr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetPosition(uint a_addr, Vector2 a_pos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        // extern static Vector2 GetSize(uint a_addr);
        extern static void GetSize(uint a_addr, IntPtr a_ptr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetSize(uint a_addr, Vector2 a_size);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector4 GetColor(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetColor(uint a_addr, Vector4 a_color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetElementState(uint a_addr);

        static readonly ConcurrentDictionary<uint, UIElement> s_elementLookup = new ConcurrentDictionary<uint, UIElement>();

        public delegate void UIEvent(Canvas a_canvas, UIElement a_element);

        string m_name;
        bool   m_destroyed = false;

        public UIEvent OnNormal = null;
        public UIEvent OnHover = null;
        public UIEvent OnPressed = null;
        public UIEvent OnReleased = null;

        public bool IsDisposed
        {
            get
            {
                return m_destroyed;
            }
        }

        internal uint BufferAddr
        {
            get;
            set;
        }

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

        public ElementState ElementState
        {
            get
            {
                return (ElementState)GetElementState(BufferAddr);
            }
        }

        public Vector2 Position
        {
            get
            {
                IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector2>());

                GetPosition(BufferAddr, ptr);
                Vector2 val = Marshal.PtrToStructure<Vector2>(ptr);
                Marshal.FreeHGlobal(ptr);

                return val;
            }
            set
            {
                SetPosition(BufferAddr, value);
            }
        }
        public Vector2 Size
        {
            get
            {
                IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf<Vector2>());

                GetSize(BufferAddr, ptr);
                Vector2 val = Marshal.PtrToStructure<Vector2>(ptr);
                Marshal.FreeHGlobal(ptr);

                return val;
                // return GetSize(BufferAddr);
                // return Vector2.One;
            }
            set
            {
                SetSize(BufferAddr, value);
            }
        }

        public Color Color
        {
            get
            {
                return GetColor(BufferAddr).ToColor();
            }
            set
            {
                SetColor(BufferAddr, value.ToVector4());
            }
        }

        public IEnumerable<UIElement> Children
        {
            get
            {
                uint[] children = GetChildren(BufferAddr);

                foreach (uint child in children)
                {
                    if (s_elementLookup.ContainsKey(child))
                    {
                        yield return s_elementLookup[child];
                    }
                }
            }
        }

        protected void AddLookup(uint a_addr, UIElement a_element)
        {
            s_elementLookup.TryAdd(a_addr, a_element);
        }
        protected void RemoveLookup(uint a_addr)
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
        public T GetNamedChild<T>(string a_name) where T : UIElement
        {
            return GetNamedChild(a_name) as T;
        }

        public void AddChild(UIElement a_child)
        {
            if (a_child == null)
            {
                Logger.IcarianWarning("Null UIElement");

                return;
            }

            AddChildElement(BufferAddr, a_child.BufferAddr);
        }
        public void RemoveChild(UIElement a_child)
        {
            if (a_child == null)
            {
                Logger.IcarianWarning("Null UIElement");
            }

            RemoveChildElement(BufferAddr, a_child.BufferAddr);
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if (!m_destroyed)
            {
                if (a_disposing)
                {
                    RemoveLookup(BufferAddr);

                    foreach (UIElement child in Children)
                    {
                        child.Dispose();
                    }
                }
                else
                {
                    Logger.IcarianWarning("UIElement failed to Dispose");
                }

                m_destroyed = true;
            }
            else
            {
                Logger.IcarianWarning("Multiple Dispose calls on UIElement");
            }
        }

        ~UIElement()
        {
            Dispose(false);
        }
    }
}