// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineTextureSamplerInteropStructures.h"

namespace IcarianEngine.Rendering
{
    // Type exists for Vulkan in the engine
    // May not work properly in the editor due to editor using OpenGL
    public class TextureSampler : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateTextureSampler(uint a_texture, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTextureSampler(uint a_renderTexture, uint a_textureIndex, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTextureDepthSampler(uint a_renderTexture, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTextureDepthSamplerDepth(uint a_renderTexture, uint a_filter, uint a_addressMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroySampler(uint a_addr);

        static ConcurrentDictionary<uint, TextureSampler> s_samplerLookup = new ConcurrentDictionary<uint, TextureSampler>();

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Returns true if the sampler has been disposed
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

        internal static TextureSampler GetSampler(uint a_bufferAddr)
        {
            TextureSampler sampler;
            if (s_samplerLookup.TryGetValue(a_bufferAddr, out sampler))
            {
                return sampler;
            }

            return null;
        }

        TextureSampler(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_samplerLookup.TryAdd(a_bufferAddr, this);
        }

        /// <summary>
        /// Generates a texture sampler from a texture
        /// </summary>
        /// <param name="a_texture">Texture to generate sampler from</param>
        /// <param name="a_filter">Filter to use for the sampler</param>
        /// <param name="a_addressMode">Address mode to use for the sampler</param>
        /// <returns>Texture sampler. Null on failure.</returns>
        public static TextureSampler GenerateTextureSampler(Texture a_texture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_texture == null)
            {
                Logger.IcarianWarning("GeneretateTextureSampler null Texture");

                return null;
            }

            return new TextureSampler(GenerateTextureSampler(a_texture.BufferAddr, (uint)a_filter, (uint) a_addressMode));
        }
        /// <summary>
        /// Generates a texture sampler from a render texture
        /// </summary>
        /// <param name="a_renderTexture">Render texture to generate sampler from</param>
        /// <param name="a_filter">Filter to use for the sampler</param>
        /// <param name="a_addressMode">Address mode to use for the sampler</param>
        /// <returns>Texture sampler. Null on failure.</returns>
        public static TextureSampler GenerateRenderTextureSampler(RenderTexture a_renderTexture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.IcarianWarning("GenerateRenderTextureSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureSampler(a_renderTexture.BufferAddr, 0, (uint)a_filter, (uint)a_addressMode));
        }
        /// <summary>
        /// Generates a texture sampler from a render texture
        /// </summary>
        /// <param name="a_renderTexture">Render texture to generate sampler from</param>
        /// <param name="a_index">Index of the texture to generate sampler from</param>
        /// <param name="a_filter">Filter to use for the sampler</param>
        /// <param name="a_addressMode">Address mode to use for the sampler</param>
        /// <returns>Texture sampler. Null on failure.</returns>
        public static TextureSampler GenerateRenderTextureSampler(MultiRenderTexture a_renderTexture, uint a_index, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.IcarianWarning("GenerateRenderTextureSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureSampler(a_renderTexture.BufferAddr, a_index, (uint)a_filter, (uint)a_addressMode));
        }
        /// <summary>
        /// Generates a texture sampler from a render texture
        /// </summary>
        /// <param name="a_renderTexture">Render texture to generate sampler from</param>
        /// <param name="a_filter">Filter to use for the sampler</param>
        /// <param name="a_addressMode">Address mode to use for the sampler</param>
        /// <returns>Texture sampler for depth. Null on failure.</returns>
        public static TextureSampler GenerateRenderTextureDepthSampler(IRenderTexture a_renderTexture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.IcarianWarning("GenerateRenderTextureDepthSampler null RenderTexture");

                return null;
            }
            
            if (!a_renderTexture.HasDepth)
            {
                Logger.IcarianWarning("GenerateRenderTextureDepthSampler no depth on render texture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureDepthSampler(RenderTextureCmd.GetTextureAddr(a_renderTexture), (uint)a_filter, (uint)a_addressMode));
        }
        /// <summary>
        /// Generates a texture sampler from a render texture
        /// </summary>
        /// <param name="a_renderTexture">Render texture to generate sampler from</param>
        /// <param name="a_filter">Filter to use for the sampler</param>
        /// <param name="a_addressMode">Address mode to use for the sampler</param>
        /// <returns>Texture sampler for depth. Null on failure.</returns>
        public static TextureSampler GenerateRenderTextureDepthSampler(DepthRenderTexture a_renderTexture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.IcarianWarning("GenerateRenderTextureDepthSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureDepthSamplerDepth(a_renderTexture.BufferAddr, (uint)a_filter, (uint)a_addressMode));
        }

        /// <summary>
        /// Disposes of the sampler
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the sampler is disposed
        /// </summary>
        /// <param name="a_disposing">True if the sampler is being disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroySampler(m_bufferAddr);

                    s_samplerLookup.TryRemove(m_bufferAddr, out TextureSampler _);
                }
                else
                {
                    Logger.IcarianWarning("TextureSampler Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple TextureSampler Dispose");
            }
        }

        ~TextureSampler()
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