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

#include "EngineCanvasInterop.h"
#include "EngineCanvasInteropStructures.h"
#include "InteropBinding.h"

ENGINE_CANVAS_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class Canvas : IDestroy
    {
        static ConcurrentDictionary<uint, Canvas> s_canvasLookup = new ConcurrentDictionary<uint, Canvas>();

        uint m_bufferAddr;

        /// <summary>
        /// Whether or not the Canvas has been Disposed/Finalised
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
        }

        /// <summary>
        /// The reference resolution of the Canvas
        /// </summary>
        public Vector2 ReferenceResolution
        {
            get
            {
                CanvasBuffer buffer = CanvasInterop.GetBuffer(m_bufferAddr);

                return buffer.ReferenceResolution;
            }
        }
        /// <summary>
        /// Whether or not the Canvas captures input
        /// </summary>
        public bool CapturesInput
        {
            get
            {
                CanvasBuffer buffer = CanvasInterop.GetBuffer(m_bufferAddr);

                return (buffer.Flags & 0b1 << (int)CanvasBuffer.CaptureInputBit) != 0;
            }
            set
            {
                CanvasBuffer buffer = CanvasInterop.GetBuffer(m_bufferAddr);

                unchecked
                {
                    bool state = (buffer.Flags & 0b1 << (int)CanvasBuffer.CaptureInputBit) != 0;
                    if (state != value)
                    {
                        if (value)
                        {
                            buffer.Flags |= (byte)(0b1 << (int)CanvasBuffer.CaptureInputBit);
                        }
                        else
                        {
                            buffer.Flags &= (byte)~(0b1 << (int)CanvasBuffer.CaptureInputBit);
                        }

                        CanvasInterop.SetBuffer(m_bufferAddr, buffer);
                    }
                }
            }
        }

        /// <summary>
        /// Gets the child <see cref="IcarianEngine.Rendering.UI.UIElement" />(s)
        /// </summary>
        public IEnumerable<UIElement> Children
        {
            get
            {
                uint[] childElementAddrs = CanvasInterop.GetChildren(m_bufferAddr);
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
        public Canvas(Vector2 a_refResolution) : this(CanvasInterop.CreateCanvas(a_refResolution))
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

        static bool SetBaseAttributes(UIElement a_element, XmlAttribute a_attribute)
        {
            switch (a_attribute.Name.ToLower())
            {
            case "name":
            {
                a_element.Name = a_attribute.Value;

                return true;
            }
            case "xpos":
            {
                float val;
                if (float.TryParse(a_attribute.Value, out val))
                {
                    a_element.Position = new Vector2(val, a_element.Position.Y);
                }
                else
                {
                    Logger.IcarianWarning($"Failed to parse xpos: {a_attribute.Value}");
                }

                return true;
            }
            case "ypos":
            {
                float val;
                if (float.TryParse(a_attribute.Value, out val))
                {
                    a_element.Position = new Vector2(a_element.Position.X, val);
                }
                else
                {
                    Logger.IcarianWarning($"Failed to parse ypos: {a_attribute.Value}");
                }

                return true;
            }
            case "width":
            {
                float val;
                if (float.TryParse(a_attribute.Value, out val))
                {
                    a_element.Size = new Vector2(val, a_element.Size.Y);
                }
                else
                {
                    Logger.IcarianWarning($"Failed to parse width: {a_attribute.Value}");
                }

                return true;
            }
            case "height":
            {
                float val;
                if (float.TryParse(a_attribute.Value, out val))
                {
                    a_element.Size = new Vector2(a_element.Size.X, val);
                }
                else
                {
                    Logger.IcarianWarning($"Failed to parse height: {a_attribute.Value}");
                }

                return true;
            }
            case "xanchor":
            {
                UIXAnchor anchor;
                if (Enum.TryParse<UIXAnchor>(a_attribute.Value, true, out anchor))
                {
                    a_element.XAnchor = anchor;
                }
                else
                {
                    Logger.IcarianWarning($"Failed to parse X anchor: {a_attribute.Value}");
                }

                return true;
            }
            case "yanchor":
            {
                UIYAnchor anchor;
                if (Enum.TryParse<UIYAnchor>(a_attribute.Value, true, out anchor))
                {
                    a_element.YAnchor = anchor;
                }
                else
                {
                    Logger.IcarianWarning($"Failed to parse X anchor: {a_attribute.Value}");
                }

                return true;
            }
            case "onnormal":
            {
                UIElement.UIEvent normalEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribute.Value);
                if (normalEvent != null)
                {
                    a_element.OnNormal += normalEvent;
                }
                else
                {
                    Logger.IcarianWarning($"Failed to find OnNormal event: {a_attribute.Value}");
                }

                return true;
            }
            case "onhover":
            {
                UIElement.UIEvent hoverEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribute.Value);
                if (hoverEvent != null)
                {
                    a_element.OnHover += hoverEvent;
                }
                else
                {
                    Logger.IcarianWarning($"Failed to find OnHover event: {a_attribute.Value}");
                }

                return true;
            }
            case "onpressed":
            {
                UIElement.UIEvent pressedEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribute.Value);
                if (pressedEvent != null)
                {
                    a_element.OnPressed += pressedEvent;
                }
                else
                {
                    Logger.IcarianWarning($"Failed to find OnPressed event: {a_attribute.Value}");
                }

                return true;
            }
            case "onreleased":
            {
                UIElement.UIEvent releasedEvent = ModControl.GetFunction<UIElement.UIEvent>(a_attribute.Value);
                if (releasedEvent != null)
                {
                    a_element.OnReleased += releasedEvent;
                }
                else
                {
                    Logger.IcarianWarning($"Failed to find OnReleased event: {a_attribute.Value}");
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
            case "element":
            {
                baseElement = new UIElement();

                foreach (XmlAttribute att in a_element.Attributes)
                {
                    SetBaseAttributes(baseElement, att);
                }

                break;
            }
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
                                scribeFont = Scribe.GetFont(text);
                                text = Scribe.GetString(text);
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
                                Logger.IcarianWarning($"Failed to parse fontsize: {att.Value}");
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
                                Logger.IcarianWarning($"Failed to load font: {att.Value}");
                            }

                            break;
                        }
                        default:
                        {
                            Logger.IcarianWarning($"Unknown TextUIElement attribute: {att.Name}");

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
                                Logger.IcarianWarning($"Failed to parse AddressMode: {att.Value}");
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
                                Logger.IcarianWarning($"Failed to parse FilterMode: {att.Value}");
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

        /// <summary>
        /// Loads a Canvas from path
        /// </summary>
        /// <param name="a_path">The path to load the Canvas from in Assets</param>
        /// <returns>The Canvas. Null on Failure</returns>
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
                        uint canvasAddr = CanvasInterop.CreateCanvas(refResolution);

                        foreach (XmlNode node in root.ChildNodes)
                        {
                            if (node is XmlElement element)
                            {
                                UIElement uiElement = UIElementFromNode(element);

                                if (uiElement != null)
                                {
                                    CanvasInterop.AddChildElement(canvasAddr, uiElement.BufferAddr);
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

        /// <summary>
        /// Adds a child <see cref="IcarianEngine.Rendering.UI.UIElement" /> to the Canvas
        /// </summary>
        /// <param name="a_element">The <see cref="IcarianEngine.Rendering.UI.UIElement" /> to add as a child</param>
        public void AddChild(UIElement a_element)
        {
            if (a_element == null)
            {
                Logger.IcarianWarning("Adding null UIElement");

                return;
            }

            CanvasInterop.AddChildElement(m_bufferAddr, a_element.BufferAddr);
        }
        /// <summary>
        /// Removes a child <see cref="IcarianEngine.Rendering.UI.UIElement" /> from the Canvas
        /// </summary>
        /// <param name="a_element">The <see cref="IcarianEngine.Rendering.UI.UIElement" /> to remove as a child</param>
        public void RemoveChild(UIElement a_element)
        {
            if (a_element == null)
            {
                Logger.IcarianWarning("Removing null UIElement");

                return;
            }

            CanvasInterop.RemoveChildElement(m_bufferAddr, a_element.BufferAddr);
        }

        /// <summary>
        /// Recursively gets a named child <see cref="IcarianEngine.Rendering.UI.UIElement" />
        /// </summary>
        /// <param name="a_name">The name of the <see cref="IcarianEngine.Rendering.UI.UIElement" /> to find</param>
        /// <returns>The <see cref="IcarianEngine.Rendering.UI.UIElement" />. Null on failure</returns>
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
        /// Recursively gets a named child <see cref="IcarianEngine.Rendering.UI.UIElement" /> of type T
        /// </summary>
        /// <param name="a_name">The name of the <see cref="IcarianEngine.Rendering.UI.UIElement" /> to find of type T</param>
        /// <returns>The <see cref="IcarianEngine.Rendering.UI.UIElement" /> of type T. Null on failure</returns>
        public T GetNamedChild<T>(string a_name) where T : UIElement
        {
            return GetNamedChild(a_name) as T;
        }

        /// <summary>
        /// Disposes of the Canvas
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Canvas is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether or not it is called from Dispose</param>
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

                    CanvasInterop.DestroyCanvas(m_bufferAddr);
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