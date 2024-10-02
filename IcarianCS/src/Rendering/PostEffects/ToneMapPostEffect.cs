// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System;

namespace IcarianEngine.Rendering.PostEffects
{
    public class ToneMapPostEffect : PostEffect, IDestroy
    {
        Vector4      m_data;

        VertexShader m_quadVertex;
        PixelShader  m_toneMapPixel;

        Material     m_material;

        /// <summary>
        /// The exposure of the ToneMap
        /// </summary>
        public float Exposure
        {
            get
            {
                return m_data.X;
            }
            set
            {
                if (m_data.X != value)
                {
                    m_data.X = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }
        /// <summary>
        /// The gamma correction of the ToneMap
        /// </summary>
        public float Gamma
        {
            get
            {
                return m_data.Y;
            }
            set
            {
                if (m_data.Y != value)
                {
                    m_data.Y = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }

        /// <summary>
        /// Whether or not the ToneMapPostEffect has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_material == null;
            }
        }

        public ToneMapPostEffect()
        {
            m_data = new Vector4(1.5f, 1.2f, 0.0f, 0.0f);

            m_quadVertex = VertexShader.LoadVertexShader("[INTERNAL]Quad");
            m_toneMapPixel = PixelShader.LoadPixelShader("[INTERNAL]PostToneMap");

            MaterialBuilder material = new MaterialBuilder()
            {
                VertexShader = m_quadVertex,
                PixelShader = m_toneMapPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None,
                UBOBuffer = m_data
            };

            m_material = Material.CreateMaterial(material);
        }

        /// <summary>
        /// Called when the post effect need to be run
        /// </summary>
        /// <param name="a_renderTexture">The target <see cref="IcarianEngine.Rendering.IRenderTexture" /></param>
        /// <param name="a_samplers">Samplers used by the RenderPipeline</param>
        /// <param name="a_gBuffer">The Deffered <see cref="IcarianEngine.Rendering.MultiRenderTexture" /> used for rendering</param>
        public override void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers, MultiRenderTexture a_gBuffer)
        {
            RenderCommand.BindRenderTexture(a_renderTexture);
            RenderCommand.BindMaterial(m_material);

            RenderCommand.PushTexture(0, a_samplers[0]);

            RenderCommand.DrawMaterial();
        }

        /// <summary>
        /// Disposes of the ToneMapPostEffect
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the ToneMapPostEffect is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether it is being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_material != null)
            {
                if (a_disposing)
                {
                    m_material.Dispose();

                    m_quadVertex.Dispose();
                    m_toneMapPixel.Dispose();
                }
                else
                {
                    Logger.IcarianWarning("ToneMapPostEffect Failed to Dispose");
                }

                m_material = null;
            }
            else
            {
                Logger.IcarianError("Multiple ToneMapPostEffect Dispose");
            }
        }
        ~ToneMapPostEffect()
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