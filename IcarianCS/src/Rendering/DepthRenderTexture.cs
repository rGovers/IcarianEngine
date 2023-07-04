using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class DepthRenderTexture : IRenderTexture
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

        static ConcurrentDictionary<uint, DepthRenderTexture> s_bufferLookup = new ConcurrentDictionary<uint, DepthRenderTexture>();

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

        public uint Width
        {
            get
            {
                return GetWidth(m_bufferAddr);
            }
        }
        public uint Height
        {
            get
            {
                return GetHeight(m_bufferAddr);
            }
        }

        public bool HasDepth
        {
            get
            {
                return true;
            }
        }

        public DepthRenderTexture(uint a_width, uint a_height)
        {
            m_bufferAddr = GenerateRenderTexture(a_width, a_height);

            s_bufferLookup.TryAdd(m_bufferAddr, this);
        }

        internal static DepthRenderTexture GetDepthRenderTexture(uint a_addr)
        {
            DepthRenderTexture buffer = null;
            s_bufferLookup.TryGetValue(a_addr, out buffer);

            return buffer;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    s_bufferLookup.TryRemove(m_bufferAddr, out DepthRenderTexture _);

                    DestroyRenderTexture(m_bufferAddr);   
                }
                else
                {
                    Logger.IcarianWarning("DepthRenderTexture not disposed");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("DepthRenderTexture already disposed");
            }
        }
        ~DepthRenderTexture()
        {
            Dispose(false);
        }

        public void Resize(uint a_width, uint a_height)
        {
            Resize(m_bufferAddr, a_width, a_height);      
        }
    }
}