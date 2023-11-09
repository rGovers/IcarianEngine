using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class DepthCubeRenderTexture : IRenderTexture
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateRenderTexture(uint a_width, uint a_height);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyRenderTexture(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetWidth(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetHeight(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void Resize(uint a_addr, uint a_width, uint a_height);

        static ConcurrentDictionary<uint, DepthCubeRenderTexture> s_bufferLookup = new ConcurrentDictionary<uint, DepthCubeRenderTexture>();

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the Depth Render Texture has been disposed
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

        /// <summary>
        /// The width of the Depth Render Texture
        /// </summary>
        public uint Width
        {
            get
            {
                return GetWidth(m_bufferAddr);
            }
        }
        /// <summary>
        /// The height of the Depth Render Texture
        /// </summary>
        public uint Height
        {
            get
            {
                return GetHeight(m_bufferAddr);
            }
        }

        /// <summary>
        /// Whether or not the Depth Render Texture has depth
        /// </summary>
        public bool HasDepth
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// Creates a new Depth Render Texture
        /// </summary>
        /// <param name="a_width">The width of the Depth Render Texture</param>
        /// <param name="a_height">The height of the Depth Render Texture</param>
        public DepthCubeRenderTexture(uint a_width, uint a_height)
        {
            m_bufferAddr = GenerateRenderTexture(a_width, a_height);

            s_bufferLookup.TryAdd(m_bufferAddr, this);
        }

        internal static DepthCubeRenderTexture GetDepthCubeRenderTexture(uint a_addr)
        {
            DepthCubeRenderTexture buffer = null;
            s_bufferLookup.TryGetValue(a_addr, out buffer);

            return buffer;
        }

        /// <summary>
        /// Resizes the Depth Render Texture
        /// </summary>
        /// <param name="a_width">The new width of the Depth Render Texture</param>
        /// <param name="a_height">The new height of the Depth Render Texture</param>
        public void Resize(uint a_width, uint a_height)
        {
            Resize(m_bufferAddr, a_width, a_height);
        }

        /// <summary>
        /// Disposes of the Depth Render Texture
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            
            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Depth Render Texture is disposed
        /// </summary>
        /// <param name="a_disposing">Whether or not the Depth Render Texture is being disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    DestroyRenderTexture(m_bufferAddr);

                    s_bufferLookup.TryRemove(m_bufferAddr, out DepthCubeRenderTexture _);
                }
                else
                {
                    Logger.IcarianWarning("DepthCubeRenderTexture not disposed");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("DepthCubeRenderTexture already disposed");
            }
        }
        ~DepthCubeRenderTexture()
        {
            Dispose(false);
        }
    }
}