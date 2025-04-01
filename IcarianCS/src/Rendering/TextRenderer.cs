// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;

namespace IcarianEngine.Rendering
{
    public class TextRenderer : Renderer, IDestroy
    {
        // NOTE: I need to see how it is used before I make the judgement but there is the case that could be made on building a cache for the TextRenderer models
        bool     m_disposed = false;

        bool     m_visible = true;

        uint     m_bufferAddr = uint.MaxValue;

        float    m_fontSize = 24.0f;
        float    m_textScale = 1.0f;
        float    m_textDepth = 0.25f;

        string   m_text;
        Font     m_font;

        Model    m_model;
        Material m_material;

        /// <summary>
        /// Whether the TextRenderer has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The Def used to create this TextRenderer
        /// </summary>
        public TextRendererDef TextRendererDef
        {
            get
            {
                return Def as TextRendererDef;
            }
        }

        /// <summary>
        /// Whether the TextRenderer is visible
        /// </summary>
        public override bool Visible
        {
            get
            {
                return m_visible;
            }
            set
            {
                if (m_visible != value)
                {
                    if (m_visible && m_bufferAddr != uint.MaxValue)
                    {
                        MeshRenderer.DestroyRenderStack(m_bufferAddr);
                    }

                    m_visible = value;

                    if (m_visible && m_bufferAddr != uint.MaxValue)
                    {
                        MeshRenderer.GenerateRenderStack(m_bufferAddr);
                    }
                }
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Font" /> used
        /// </summary>
        public Font Font
        {
            get
            {
                return m_font;
            }
            set
            {
                if (m_font != value)
                {
                    m_font = value;

                    UpdateModel();
                }
            }
        }

        /// <summary>
        /// The size of the <see cref="IcarianEngine.Rendering.Font" />
        /// </summary>
        public float FontSize
        {
            get
            {
                return m_fontSize;
            }
            set
            {
                if (m_fontSize != value)
                {
                    m_fontSize = value;

                    UpdateModel();
                }
            }
        }

        /// <summary>
        /// The ammount to scale the text by
        /// </summary>
        public float TextScale
        {
            get
            {
                return m_textScale;
            }
            set
            {
                if (m_textScale != value)
                {
                    m_textScale = value;

                    UpdateModel();
                }
            }
        }

        /// <summary>
        /// The depth of the text
        /// </summary>
        public float TextDepth
        {
            get
            {
                return m_textDepth;
            }
            set
            {
                m_textDepth = value;

                UpdateModel();
            }
        }

        /// <summary>
        /// The text of the TextRenderer
        /// </summary>
        public string Text
        {
            get
            {
                return m_text;
            }
            set
            {
                if (m_text != value)
                {
                    m_text = value;
                
                    UpdateModel();
                }
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Material" /> of the TextRenderer
        /// </summary>
        public override Material Material
        {
            get
            {
                return m_material;
            }
            set
            {
                if (m_material != value)
                {
                    m_material = value;
                    
                    UpdateRenderer();
                }
            }
        }

        void AddRenderer()
        {
            if (m_model != null && m_material != null)
            {
                m_bufferAddr = MeshRenderer.GenerateBuffer(Transform.InternalAddr, m_material.InternalAddr, m_model.InternalAddr);

                if (m_visible)
                {
                    MeshRenderer.GenerateRenderStack(m_bufferAddr);
                }
            }
        }
        void ClearRenderer()
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (m_visible)
                {
                    MeshRenderer.DestroyRenderStack(m_bufferAddr);
                }

                MeshRenderer.DestroyBuffer(m_bufferAddr);

                m_bufferAddr = uint.MaxValue;
            }
        }

        void UpdateModel()
        {
            ClearRenderer();

            if (m_model != null)
            {
                m_model.Dispose();
                m_model = null;
            }

            if (m_font != null && !string.IsNullOrEmpty(m_text))
            {
                m_model = m_font.CreateModel(m_text, m_fontSize, m_textScale, m_textDepth);
            }

            AddRenderer();
        }
        void UpdateRenderer()
        {
            ClearRenderer();

            AddRenderer();
        }

        /// <summary>
        /// Called when the TextRenderer is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            RendererDef def = RendererDef;
            if (def != null)
            {
                Material = AssetLibrary.GetMaterial(def.MaterialDef);

                TextRendererDef textDef = TextRendererDef;
                if (textDef != null)
                {
                    string text = textDef.Text;
                    if (Scribe.StringKeyExists(text))
                    {
                        text = Scribe.GetString(text);
                    }

                    Text = text;
                    FontSize = textDef.FontSize;
                    TextScale = textDef.TextScale;
                    TextDepth = textDef.TextDepth;

                    if (!string.IsNullOrWhiteSpace(textDef.FontPath))
                    {
                        Font = AssetLibrary.LoadFont(textDef.FontPath);
                    }
                }
            }
        }

        /// <summary>
        /// Disposes of the TextRenderer
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the TextRenderer is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether is was called from Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (!m_disposed)
            {
                if (a_disposing)
                {
                    ClearRenderer();

                    if (m_model != null)
                    {
                        m_model.Dispose();
                        m_model = null;
                    }
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("Multiple TextRenderer Dispose");
            }
        }
        ~TextRenderer()
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