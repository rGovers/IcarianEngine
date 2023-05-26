using IcarianEngine.Mod;
using IcarianEngine.Maths;
using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Xml;
using System.Collections.Generic;

namespace IcarianEngine.Rendering.UI
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    struct CanvasBuffer
    {
        public Vector2 ReferenceResolution;
        public uint ChildElementCount;
        public IntPtr ChildElements;
        public byte Flags;
    }

    public class Canvas : IDestroy
    {
        const int CaptureInputBit = 1;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateCanvas(Vector2 a_refResolution);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyCanvas(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static CanvasBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, CanvasBuffer a_buffer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddChildElement(uint a_addr, uint a_childElementAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RemoveChildElement(uint a_addr, uint a_childElementAddr);

        uint m_bufferAddr;

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        public Vector2 ReferenceResolution
        {
            get
            {
                CanvasBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.ReferenceResolution;
            }
        }
        public bool CapturesInput
        {
            get
            {
                CanvasBuffer buffer = GetBuffer(m_bufferAddr);

                return (buffer.Flags & 0b1 << CaptureInputBit) != 0;
            }
            set
            {
                CanvasBuffer buffer = GetBuffer(m_bufferAddr);

                if (value)
                {
                    buffer.Flags |= 0b1 << CaptureInputBit;
                }
                else
                {
                    buffer.Flags = (byte)(buffer.Flags & ~(0b1 << CaptureInputBit));
                }

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public IEnumerable<UIElement> Children
        {
            get
            {
                CanvasBuffer buffer = GetBuffer(m_bufferAddr);

                // C# forcing a copy and conversion in the name of "Memory Safety"
                // cause apparently if this is to be believed doing a copy makes it "safe"
                // dont know where they are getting that idea from garbage is garbage
                byte[] bytes = new byte[buffer.ChildElementCount * sizeof(uint)];
                Marshal.Copy(buffer.ChildElements, bytes, 0, (int)buffer.ChildElementCount * sizeof(uint));

                UIElement[] elements = new UIElement[buffer.ChildElementCount];
                for (uint i = 0; i < buffer.ChildElementCount; ++i)
                {
                    elements[i] = UIElement.GetUIElement(BitConverter.ToUInt32(bytes, (int)i * sizeof(uint)));
                }

                return elements;
            }
        }

        private Canvas(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;
        }

        static bool SetBaseAttributes(UIElement a_element, XmlAttribute a_attribue)
        {
            switch (a_attribue.Name.ToLower())
            {
            case "xpos":
            {
                break;
            }
            case "ypos":
            {
                break;
            }
            case "width":
            {
                break;
            }
            case "height":
            {
                break;
            }
            }

            return false;
        }

        static UIElement UIElementFromNode(XmlElement a_element)
        {
            UIElement baseElement = null;
            switch (a_element.Name.ToLower())
            {
            case "text":
            {
                TextUIElement textElement = new TextUIElement();
                baseElement = textElement;

                foreach (XmlAttribute att in a_element.Attributes)
                {
                    if (!SetBaseAttributes(baseElement, att))
                    {
                        switch (att.Name.ToLower())
                        {
                        case "text":
                        {
                            break;
                        }
                        }
                    }
                }

                break;
            }
            }

            if (baseElement != null)
            {
                foreach (XmlNode node in a_element.ChildNodes)
                {
                    if (node is XmlElement element)
                    {
                        UIElement childElement = UIElementFromNode(element);

                        baseElement.AddChild(childElement);
                    }
                }
            }

            return baseElement;
        }

        public static Canvas FromFile(string a_path)
        {
            string filePath = ModControl.GetAssetPath(a_path);
            if (File.Exists(filePath))
            {
                XmlDocument doc = new XmlDocument();
                doc.Load(filePath);

                if (doc.DocumentElement is XmlElement root)
                {
                    Vector2 refResolution = Vector2.Zero;

                    foreach (XmlAttribute att in root.Attributes)
                    {
                        switch (att.Name.ToLower())
                        {
                        case "xres":
                        {
                            float val; 
                            if (float.TryParse(att.Value, out val))
                            {
                                refResolution.X = val;   
                            }

                            break;
                        }
                        case "yres":
                        {
                            float val;
                            if (float.TryParse(att.Value, out val))
                            {
                                refResolution.Y = val;
                            }

                            break;
                        }
                        }
                    }

                    if (refResolution.X > 0 && refResolution.Y > 0)
                    {
                        uint canvasAddr = CreateCanvas(refResolution);

                        foreach (XmlNode node in root.ChildNodes)
                        {
                            if (node is XmlElement element)
                            {
                                UIElement uiElement = UIElementFromNode(element);

                                if (uiElement != null)
                                {
                                    AddChildElement(canvasAddr, uiElement.BufferAddr);
                                }
                            }
                        }

                        return new Canvas(canvasAddr);
                    }
                    else
                    {
                        Logger.IcarianWarning($"Canvas invalid resolution: {a_path}: {filePath}");
                    }
                }
                else
                {
                    Logger.IcarianWarning($"Canvas file empty: {a_path}: {filePath}");
                }
            }
            else
            {
                Logger.IcarianError($"Canvas file does not exist: {a_path}: {filePath}");
            }

            return null;
        }

        public void AddChild(UIElement a_element)
        {
            if (a_element == null)
            {
                Logger.IcarianWarning("Adding null UIElement");

                return;
            }

            AddChildElement(m_bufferAddr, a_element.BufferAddr);
        }
        public void RemoveChild(UIElement a_element)
        {
            if (a_element == null)
            {
                Logger.IcarianWarning("Removing null UIElement");

                return;
            }

            RemoveChildElement(m_bufferAddr, a_element.BufferAddr);
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyCanvas(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Canvas Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Canvas Dispose");
            }
        }

        ~Canvas()
        {
            Dispose(false);
        }
    }
}