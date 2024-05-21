using IcarianEngine.Maths;
using System.Runtime.CompilerServices;

#include "EngineRenderCommandInteropStructures.h"

namespace IcarianEngine.Rendering
{
    public static class RenderCommand
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindMaterial(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushTexture(uint a_slot, uint a_sampler);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindRenderTexture(uint a_addr, uint a_bindMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RTRTBlit(uint a_srcAddr, uint a_dstAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DrawModel(float[] a_transform, uint a_modelAddr);

        /// <summary>
        /// Draws the currently bound <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        /// Renders the currently bound <see cref="IcarianEngine.Rendering.Material" /> to the currently bound <see cref="IcarianEngine.Rendering.IRenderTexture" />
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void DrawMaterial();

        /// <summary>
        /// Binds a <see cref="IcarianEngine.Rendering.Material" /> for rendering in a renderpass
        /// </summary>
        /// <param name="a_material">The <see cref="IcarianEngine.Rendering.Material" /> to bind</param>
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

        /// <summary>
        /// Sets a <see cref="IcarianEngine.Rendering.ShaderBufferType.PushTexture" /> for the current render state
        /// </summary>
        /// <param name="a_slot">The shader slot to bind the <see cref="IcarianEngine.Rendering.TextureSampler" /> to</param>
        /// <param name="a_sampler">The <see cref="IcarianEngine.Rendering.TextureSampler" /> to bind to the slot</param>
        public static void PushTexture(uint a_slot, TextureSampler a_sampler)
        {
            if (a_sampler == null)
            {
                Logger.IcarianWarning("PushTexture null sampler");

                return;
            }

            PushTexture(a_slot, a_sampler.BufferAddr);
        }

        /// <summary>
        /// Binds a RenderTexture for rendering to
        /// </summary>
        /// <param name="a_slot">The <see cref="IcarianEngine.Rendering.IRenderTexture" /> to bind. Null binds the swapchain</param>
        /// <param name="a_bindMode">The clear mode to use when binding</param>
        public static void BindRenderTexture(IRenderTexture a_renderTexture, RenderTextureBindMode a_bindMode = RenderTextureBindMode.Clear)
        {
            BindRenderTexture(RenderTextureCmd.GetTextureAddr(a_renderTexture), (uint)a_bindMode);
        }
        /// <summary>
        /// Copies the contents of a <see cref="IcarianEngine.Rendering.IRenderTexture" /> to another <see cref="IcarianEngine.Rendering.IRenderTexture" />
        /// </summary>
        /// <param name="a_srcTexture">The <see cref="IcarianEngine.Rendering.IRenderTexture" /> to use as the source</param>
        /// <param name="a_dstTexture">The <see cref="IcarianEngine.Rendering.IRenderTexture" /> to use as the destination. Null writes to the swapchain</param>
        public static void Blit(IRenderTexture a_srcTexture, IRenderTexture a_dstTexture)
        {
            RTRTBlit(RenderTextureCmd.GetTextureAddr(a_srcTexture), RenderTextureCmd.GetTextureAddr(a_dstTexture));
        }
        /// <summary>
        /// Draws a Model
        /// </summary>
        /// <param name="a_transform">The transformation matrix to use for the model</param>
        /// <param name="a_model">The model to render</param>
        /// Renders a model with the currently bound <see cref="IcarianEngine.Rendering.Material" /> to the currently bound <see cref="IcarianEngine.Rendering.IRenderTexture" />
        public static void DrawModel(Matrix4 a_transform, Model a_model)
        {
            DrawModel(a_transform.ToArray(), a_model.InternalAddr);
        }
    }
}