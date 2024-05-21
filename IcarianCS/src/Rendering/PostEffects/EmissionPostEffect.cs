using System;

namespace IcarianEngine.Rendering.PostEffects
{
    public class EmissionPostEffect : PostEffect, IDestroy
    {
        bool         m_disposed;

        VertexShader m_quadVertex;
        PixelShader  m_emissionPixel;

        Material     m_material;

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public EmissionPostEffect()
        {
            m_disposed = false;

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

        public override void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers)
        {
            RenderCommand.BindRenderTexture(a_renderTexture);
            RenderCommand.BindMaterial(m_material);

            RenderCommand.PushTexture(0, a_samplers[0]);
            RenderCommand.PushTexture(1, a_samplers[2]);

            RenderCommand.DrawMaterial();
        }   

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
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

                m_disposed = true;
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