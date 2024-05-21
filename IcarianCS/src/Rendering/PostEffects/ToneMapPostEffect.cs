using System;

namespace IcarianEngine.Rendering.PostEffects
{
    public class ToneMapPostEffect : PostEffect, IDestroy
    {
        bool         m_disposed;

        VertexShader m_quadVertex;
        PixelShader  m_toneMapPixel;

        Material     m_material;

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public ToneMapPostEffect()
        {
            m_disposed = false;

            m_quadVertex = VertexShader.LoadVertexShader("[INTERNAL]Quad");
            m_toneMapPixel = PixelShader.LoadPixelShader("[INTERNAL]PostToneMap");

            MaterialBuilder material = new MaterialBuilder()
            {
                VertexShader = m_quadVertex,
                PixelShader = m_toneMapPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None
            };

            m_material = Material.CreateMaterial(material);
        }

        public override void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers)
        {
            RenderCommand.BindRenderTexture(a_renderTexture);
            RenderCommand.BindMaterial(m_material);

            RenderCommand.PushTexture(0, a_samplers[0]);

            RenderCommand.DrawMaterial();
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool a_disposing)
        {
            if (!m_disposed)
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

                m_disposed = true;
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