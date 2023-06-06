using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public enum TextureFilter : ushort
    {
        Nearest = 0,
        Linear = 1
    }
    public enum TextureAddress : ushort
    {
        Repeat = 0,
        MirroredRepeat = 1,
        ClampToEdge = 2
    };

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
        extern static void DestroySampler(uint a_addr);

        static ConcurrentDictionary<uint, TextureSampler> s_samplerLookup = new ConcurrentDictionary<uint, TextureSampler>();

        uint m_bufferAddr = uint.MaxValue;

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

        public static TextureSampler GeneretateTextureSampler(Texture a_texture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_texture == null)
            {
                Logger.IcarianWarning("GeneretateTextureSampler null Texture");

                return null;
            }

            return new TextureSampler(GenerateTextureSampler(a_texture.BufferAddr, (uint)a_filter, (uint) a_addressMode));
        }
        public static TextureSampler GenerateRenderTextureSampler(RenderTexture a_renderTexture, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.IcarianWarning("GenerateRenderTextureSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureSampler(a_renderTexture.BufferAddr, 0, (uint)a_filter, (uint)a_addressMode));
        }
        public static TextureSampler GenerateRenderTextureSampler(MultiRenderTexture a_renderTexture, uint a_index, TextureFilter a_filter = TextureFilter.Linear, TextureAddress a_addressMode = TextureAddress.Repeat)
        {
            if (a_renderTexture == null)
            {
                Logger.IcarianWarning("GenerateRenderTextureSampler null RenderTexture");

                return null;
            }

            return new TextureSampler(GenerateRenderTextureSampler(a_renderTexture.BufferAddr, a_index, (uint)a_filter, (uint)a_addressMode));
        }

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

        public override bool Equals(object a_obj)
        {
            if (a_obj == null && m_bufferAddr == uint.MaxValue)
            {
                return true;
            }

            return base.Equals(a_obj);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

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