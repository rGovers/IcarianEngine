// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;

namespace IcarianEngine.Rendering.PostEffects
{
    public class EmissionPostEffect : PostEffect, IDestroy
    {
        VertexShader m_quadVertex;
        PixelShader  m_emissionPixel;

        Material     m_material;

        /// <summary>
        /// Whether or not the EmissionPostEffect has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_material == null;
            }
        }

        public EmissionPostEffect()
        {
            m_quadVertex = VertexShader.LoadVertexShader("[INTERNAL]Quad");
            m_emissionPixel = PixelShader.LoadPixelShader("[INTERNAL]PostEmission");

            MaterialBuilder material = new MaterialBuilder()
            {
                VertexShader = m_quadVertex,
                PixelShader = m_emissionPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None
            };

            m_material = Material.CreateMaterial(material);
        }

        /// <summary>
        /// Called when the post effect need to be run
        /// </summary>
        /// <param name="a_renderTexture">The target <see cref="IcarianEngine.Rendering.IRenderTexture" /></param>
        /// <param name="a_samplers">Samplers used by the RenderPipeline</param>
        public override void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers)
        {
            RenderCommand.BindRenderTexture(a_renderTexture);
            RenderCommand.BindMaterial(m_material);

            RenderCommand.PushTexture(0, a_samplers[0]);
            RenderCommand.PushTexture(1, a_samplers[2]);

            RenderCommand.DrawMaterial();
        }   

        /// <summary>
        /// Disposes of the EmissionPostEffect
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the EmissionPostEffect is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether it is being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_material != null)
            {
                if(a_disposing)
                {
                    m_material.Dispose();

                    m_quadVertex.Dispose();
                    m_emissionPixel.Dispose();
                }
                else
                {
                    Logger.IcarianWarning("EmissionPostEffect Failed to Dispose");
                }

                m_material = null;
            }
            else
            {
                Logger.IcarianError("Multiple EmissionPostEffect Dispose");
            }
        }
        ~EmissionPostEffect()
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