using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Xml;

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
        static ConcurrentDictionary<uint, Canvas> s_canvasLookup = new ConcurrentDictionary<uint, Canvas>();

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
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint[] GetChildren(uint a_addr);

        uint m_bufferAddr;

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
                uint[] childElementAddrs = GetChildren(m_bufferAddr);
                foreach (uint childElementAddr in childElementAddrs)
                {
                    UIElement element = UIElement.GetUIElement(childElementAddr);
                    if (element != null)
                    {
                        yield return element;
                    }
                }
            }
        }

        Canvas(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_canvasLookup.TryAdd(m_bufferAddr, this);
        }
        public Canvas(Vector2 a_refResolution) : this(CreateCanvas(a_refResolution))
        {

        }

        internal static Canvas GetCanvas(uint a_addr)
        {
            Canvas canvas;
            if (s_canvasLookup.TryGetValue(a_addr, out canvas))
            {
                return canvas;
            }

            return null;
        }

        static bool SetBaseAttributes(UIElement a_element, XmlAttribute a_attribue)
        {
            switch (a_attribue.Name.ToLower())
            {
            case "name":
            {
                a_element.Name = a_attribue.Value;

                return true;
            }
            case "xpos":
            {
                float val;
                if (float.TryParse(a_attribue.Value, out val))
                {
                    a_element.Position = new Vector2(val, a_element.Position.Y);
                }
                else
                {
                    Logger.IcarianError($"Failed to parse xpos: {a_attribue.Value}");
                }

                return true;
            }
            case "ypos":
            {
                float val;
                if (float.TryParse(a_attribue.Value, out val))
                {
                    a_element.Position = new Vector2(a_element.Position.X, val);
                }
                else
                {
                    Logger.IcarianError($"Failed to parse ypos: {a_attribue.Value}");
                }

                return true;
            }
            case "width":
            {
                float val;
                if (float.TryParse(a_attribue.Value, out val))
                {
                    a_element.Size = new Vector2(val, a_element.Size.Y);
                }
                else
                {
                    Logger.IcarianError($"Failed to parse width: {a_attribue.Value}");
                }

                return true;
            }
            case "height":
            {
                float val;
                if (float.TryParse(a_attribue.Value, out val))
                {
                    a_element.Size = new Vector2(a_element.Size.X, val);
                }
                else
                {
                    Logger.IcarianError($"Failed to parse height: {a_attribue.Value}");
                }

                return true;
            }
            case "onnormal":
            {
                UIElement.UIEvent normalEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribue.Value);
                if (normalEvent != null)
                {
                    a_element.OnNormal += normalEvent;
                }
                else
                {
                    Logger.IcarianError($"Failed to find OnNormal event: {a_attribue.Value}");
                }

                return true;
            }
            case "onhover":
            {
                UIElement.UIEvent hoverEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribue.Value);
                if (hoverEvent != null)
                {
                    a_element.OnHover += hoverEvent;
                }
                else
                {
                    Logger.IcarianError($"Failed to find OnHover event: {a_attribue.Value}");
                }

                return true;
            }
            case "onpressed":
            {
                UIElement.UIEvent pressedEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribue.Value);
                if (pressedEvent != null)
                {
                    a_element.OnPressed += pressedEvent;
                }
                else
                {
                    Logger.IcarianError($"Failed to find OnPressed event: {a_attribue.Value}");
                }

                return true;
            }
            case "onreleased":
            {
                UIElement.UIEvent releasedEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribue.Value);
                if (releasedEvent != null)
                {
                    a_element.OnReleased += releasedEvent;
                }
                else
                {
                    Logger.IcarianError($"Failed to find OnReleased event: {a_attribue.Value}");
                }

                return true;
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

                Font scribeFont = null;

                foreach (XmlAttribute att in a_element.Attributes)
                {
                    if (!SetBaseAttributes(baseElement, att))
                    {
                        switch (att.Name.ToLower())
                        {
                        case "text":
                        {
                            string text = att.Value;
                            if (Scribe.KeyExists(text))
                            {
                                text = Scribe.GetString(text);
                                scribeFont = Scribe.GetFont(text);
                            }

                            textElement.Text = text;

                            break;
                        }
                        case "fontsize":
                        {
                            float val;
                            if (float.TryParse(att.Value, out val))
                            {
                                textElement.FontSize = val;
                            }
                            else
                            {
                                Logger.IcarianError($"Failed to parse fontsize: {att.Value}");
                            }

                            break;
                        }
                        case "font":
                        {
                            Font font = AssetLibrary.LoadFont(att.Value);
                            if (font != null)
                            {
                                textElement.Font = font;
                            }
                            else
                            {
                                Logger.IcarianError($"Failed to load font: {att.Value}");
                            }

                            break;
                        }
                        default:
                        {
                            Logger.IcarianError($"Unknown TextUIElement attribute: {att.Name}");

                            break;
                        }
                        }
                    }
                }

                if (textElement.Font == null)
                {
                    textElement.Font = scribeFont;
                }

                break;
            }
            case "image":
            {
                ImageUIElement imageElement = new ImageUIElement();
                baseElement = imageElement;

                TextureInput input = new TextureInput()
                {
                    Slot = 0,
                    AddressMode = TextureAddress.ClampToEdge,
                    FilterMode = TextureFilter.Linear
                };

                foreach (XmlAttribute att in a_element.Attributes)
                {
                    if (!SetBaseAttributes(baseElement, att))
                    {
                        switch (att.Name.ToLower())
                        {
                        case "path":
                        {
                            input.Path = att.Value;

                            break;
                        }
                        case "addressmode":
                        {
                            object val = Enum.Parse(typeof(TextureAddress), att.Value, true);
                            if (val != null)
                            {
                                input.AddressMode = (TextureAddress)val;
                            }
                            else
                            {
                                Logger.IcarianError($"Failed to parse AddressMode: {att.Value}");
                            }

                            break;
                        }
                        case "filtermode":
                        {
                            object val = Enum.Parse(typeof(TextureFilter), att.Value, true);
                            if (val != null)
                            {
                                input.FilterMode = (TextureFilter)val;
                            }
                            else
                            {
                                Logger.IcarianError($"Failed to parse FilterMode: {att.Value}");
                            }

                            break;
                        }
                        }
                    }
                }

                if (!string.IsNullOrWhiteSpace(input.Path))
                {
                    TextureSampler sampler = AssetLibrary.GetSampler(input);
                    if (sampler != null)
                    {
                        imageElement.Sampler = sampler;
                    }
                    else
                    {
                        Logger.IcarianError($"Failed to load sampler: {input.Path}");
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
                    s_canvasLookup.TryRemove(m_bufferAddr, out Canvas _);

                    foreach (UIElement child in Children)
                    {
                        child.Dispose();
                    }

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