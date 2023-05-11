using IcarianEngine.Maths;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public static class RenderCommand
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindMaterial(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushTexture(uint a_slot, uint a_sampler);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindRenderTexture(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RTRTBlit(uint a_srcAddr, uint a_dstAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DrawModel(float[] a_transform, uint a_modelAddr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawMaterial();

        public static void BindMaterial(Material a_material)
        {
            if (a_material != null)
            {
                BindMaterial(a_material.InternalAddr);
            }
            else
            {
                BindMaterial(uint.MaxValue);
            }
        }

        public static void PushTexture(uint a_slot, TextureSampler a_sampler)
        {
            if (a_sampler == null)
            {
                Logger.IcarianWarning("PushTexture null sampler");

                return;
            }

            PushTexture(a_slot, a_sampler.BufferAddr);
        }

        public static void BindRenderTexture(IRenderTexture a_renderTexture)
        {
            BindRenderTexture(RenderTextureCmd.GetTextureAddr(a_renderTexture));
        }
        public static void Blit(IRenderTexture a_srcTexture, IRenderTexture a_dstTexture)
        {
            RTRTBlit(RenderTextureCmd.GetTextureAddr(a_srcTexture), RenderTextureCmd.GetTextureAddr(a_dstTexture));
        }
        public static void DrawModel(Matrix4 a_transform, Model a_model)
        {
            DrawModel(a_transform.ToArray(), a_model.InternalAddr);
        }
    }
}