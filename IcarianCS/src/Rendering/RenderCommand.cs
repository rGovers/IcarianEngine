// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

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
        extern static void PushLight(uint a_slot, uint a_lightType, uint a_lightAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushShadowTextureArray(uint a_slot, uint a_lightAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void BindRenderTexture(uint a_addr, uint a_bindMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RTRTBlit(uint a_srcAddr, uint a_dstAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void MTRTBlit(uint a_srcAddr, uint a_index, uint a_dstAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DrawModel(Matrix4 a_transform, uint a_modelAddr);

        /// <summary>
        /// Sets a <see cref="IcarianEngine.Rendering.ShaderBufferType.SSShadowLightBuffer" /> for the current render state
        /// </summary>
        /// <param name="a_slot">The shader slot to bind the <see cref="IcarianEngine.Rendering.LightShadowSplit" />(s) to</param>
        /// <param name="a_splits">The <see cref="IcarianEngine.Rendering.LightShadowSplit" />(s) to bind to the slot</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void PushShadowSplits(uint a_slot, LightShadowSplit[] a_splits);
        
        /// <summary>
        /// Adds a marker region for use by graphics debuggers if enabled in build settings
        /// </summary>
        /// <param name="a_name">The name of the marker</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void MarkerStart(string a_name);
        /// <summary>
        /// Ends a marker region
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void MarkerEnd();

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
        /// Sets a respective light buffer for the current render state
        /// </summary>
        /// <param name="a_slot">The shader slot to bind the <see cref="IcarianEngine.Rendering.Lighting.Light" /> to</param>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> to bind to the slot</param>
        public static void PushLight(uint a_slot, Light a_light)
        {
            if (a_light == null)
            {
                Logger.IcarianWarning("PushLight null light");

                return;
            }

            switch (a_light.LightType)
            {
            case LightType.Ambient:
            {
                AmbientLight ambientLight = (AmbientLight)a_light;

                PushLight(a_slot, (uint)LightType.Ambient, ambientLight.InternalAddr);

                break;
            }
            case LightType.Directional:
            {
                DirectionalLight dirLight = (DirectionalLight)a_light;

                PushLight(a_slot, (uint)LightType.Directional, dirLight.InternalAddr);

                break;
            }
            case LightType.Point:
            {
                PointLight pointLight = (PointLight)a_light;

                PushLight(a_slot, (uint)LightType.Point, pointLight.InternalAddr);

                break;
            }
            case LightType.Spot:
            {
                SpotLight spotLight = (SpotLight)a_light;

                PushLight(a_slot, (uint)LightType.Spot, spotLight.InternalAddr);

                break;
            }
            }
        }

        /// <summary>
        /// Sets a <see cref="IcarianEngine.Rendering.ShaderBufferType.AShadowTexture2D" /> for the current render state
        /// </summary>
        /// <param name="a_slot">The shader slot to bind the ShadowMap to</param>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.DirectionalLight" /> to use for the texture array</param>
        public static void PushShadowTextureArray(uint a_slot, DirectionalLight a_light)
        {
            if (a_light == null)
            {
                Logger.IcarianWarning("PushShadowTextureArray null light");

                return;
            }

            if (a_light.ShadowMaps == null)
            {
                Logger.IcarianWarning("PushShadowTextureArray null ShadowMaps");

                return;
            }

            PushShadowTextureArray(a_slot, a_light.InternalAddr);
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
        /// Copies the contents of a <see cref="IcarianEngine.Rendering.MultiRenderTexture" /> to another <see cref="IcarianEngine.Rendering.IRenderTexture" />
        /// </summary>
        /// <param name="a_srcTexture">The <see cref="IcarianEngine.Rendering.MultiRenderTexture" /> to use as the source</param>
        /// <param name="a_index">The index in the source to Blit</param>
        /// <param name="a_dstTexture">The <see cref="IcarianEngine.Rendering.IRenderTexture" /> to use as the destination. Null writes to the swapchain</param>
        public static void Blit(MultiRenderTexture a_srcTexture, uint a_index, IRenderTexture a_dstTexture)
        {
            MTRTBlit(a_srcTexture.BufferAddr, a_index, RenderTextureCmd.GetTextureAddr(a_dstTexture));
        }

        /// <summary>
        /// Draws a Model
        /// </summary>
        /// <param name="a_transform">The transformation matrix to use for the model</param>
        /// <param name="a_model">The model to render</param>
        /// Renders a model with the currently bound <see cref="IcarianEngine.Rendering.Material" /> to the currently bound <see cref="IcarianEngine.Rendering.IRenderTexture" />
        public static void DrawModel(Matrix4 a_transform, Model a_model)
        {
            if (a_model == null)
            {
                Logger.IcarianWarning("DrawModel null model");

                return;
            }

            DrawModel(a_transform, a_model.InternalAddr);
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