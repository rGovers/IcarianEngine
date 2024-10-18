// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;

namespace IcarianEngine.Rendering.PostEffects
{
    public class EmissionPostEffect : PostEffect, IDestroy
    {
        const uint RenderTextureCount = 4;

        VertexShader     m_quadVertex;
        PixelShader      m_emissionPixel;
        PixelShader      m_blurPixel;

        Material         m_blurMaterial;
        Material         m_emissionMaterial;
 
        RenderTexture[]  m_renderTextures;
        TextureSampler[] m_textureSamplers;

        /// <summary>
        /// Whether or not the EmissionPostEffect has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_emissionMaterial == null;
            }
        }

        public EmissionPostEffect()
        {
            m_quadVertex = VertexShader.LoadVertexShader("[INTERNAL]Quad");
            m_emissionPixel = PixelShader.LoadPixelShader("[INTERNAL]PostEmission");
            m_blurPixel = PixelShader.LoadPixelShader("[INTERNAL]PostEmissionBlur");

            MaterialBuilder material = new MaterialBuilder()
            {
                VertexShader = m_quadVertex,
                PixelShader = m_emissionPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None
            };

            MaterialBuilder blurMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVertex,
                PixelShader = m_blurPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None
            };

            m_emissionMaterial = Material.CreateMaterial(material);
            m_blurMaterial = Material.CreateMaterial(blurMaterial);

            m_renderTextures = new RenderTexture[RenderTextureCount];
            m_textureSamplers = new TextureSampler[RenderTextureCount];

            for (uint i = 0; i < RenderTextureCount; ++i)
            {
                uint next = i + 1;

                m_renderTextures[i] = new RenderTexture((uint)(1920 >> (int)next), (uint)(1080 >> (int)next), false, true);
                m_textureSamplers[i] = TextureSampler.GenerateRenderTextureSampler(m_renderTextures[i]);
            }
        }

        /// <summary>
        /// Called when the SwapChain is resized
        /// </summary>
        public override void Resize(uint a_width, uint a_height)
        {
            for (uint i = 0; i < RenderTextureCount; ++i)
            {
                uint next = i + 1;

                m_renderTextures[i].Resize((uint)(a_width >> (int)next), (uint)(a_height >> (int)next));
            }
        }

        /// <summary>
        /// Called when the EmissionPostEffect needs to be run
        /// </summary>
        /// <param name="a_renderTexture">The target <see cref="IcarianEngine.Rendering.IRenderTexture" /></param>
        /// <param name="a_samplers">Samplers used by the RenderPipeline</param>
        /// <param name="a_gBuffer">The Deffered <see cref="IcarianEngine.Rendering.MultiRenderTexture" /> used for rendering</param>
        public override void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers, MultiRenderTexture a_gBuffer)
        {
            RenderCommand.MarkerStart("Emission");

            RenderCommand.Blit(a_gBuffer, 3, m_renderTextures[0]);
            for (uint i = 1; i < RenderTextureCount; ++i)
            {
                RenderCommand.Blit(m_renderTextures[i - 1], m_renderTextures[i]);
            }

            for (int i = (int)RenderTextureCount - 2; i >= 0; --i)
            {
                RenderCommand.BindRenderTexture(m_renderTextures[i]);
                RenderCommand.BindMaterial(m_blurMaterial);

                RenderCommand.PushTexture(0, m_textureSamplers[i + 1]);

                RenderCommand.DrawMaterial();
            }

            RenderCommand.BindRenderTexture(a_renderTexture);
            RenderCommand.BindMaterial(m_emissionMaterial);

            RenderCommand.PushTexture(0, a_samplers[0]);
            RenderCommand.PushTexture(1, a_samplers[2]);
            RenderCommand.PushTexture(2, m_textureSamplers[0]);

            RenderCommand.DrawMaterial();

            RenderCommand.MarkerEnd();
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
            if(m_emissionMaterial != null)
            {
                if(a_disposing)
                {
                    for (uint i = 0; i < RenderTextureCount; ++i)
                    {
                        m_textureSamplers[i].Dispose();
                        m_renderTextures[i].Dispose();
                    }

                    m_emissionMaterial.Dispose();
                    m_blurMaterial.Dispose();

                    m_quadVertex.Dispose();
                    m_blurPixel.Dispose();
                    m_emissionPixel.Dispose();
                }
                else
                {
                    Logger.IcarianWarning("EmissionPostEffect Failed to Dispose");
                }

                m_emissionMaterial = null;
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