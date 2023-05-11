using System;

namespace IcarianEngine.Rendering
{
    public class RenderTexture : IRenderTexture
    {
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
                return RenderTextureCmd.GetWidth(m_bufferAddr);
            }
        }

        public uint Height
        {
            get
            {
                return RenderTextureCmd.GetHeight(m_bufferAddr);
            }
        }

        public bool HasDepth
        {
            get
            {
                return RenderTextureCmd.HasDepth(m_bufferAddr) != 0;
            }
        }

        public RenderTexture(uint a_width, uint a_height, bool a_depth = false, bool a_hdr = false)
        {
            uint depthVal = 0;
            if (a_depth)
            {
                depthVal = 1;
            }
            
            uint hdrVal = 0;
            if (a_hdr)
            {
                hdrVal = 1;
            }

            m_bufferAddr = RenderTextureCmd.GenerateRenderTexture(1, a_width, a_height, depthVal, hdrVal);

            RenderTextureCmd.PushRenderTexture(m_bufferAddr, this);
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
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    RenderTextureCmd.RemoveRenderTexture(m_bufferAddr);

                    RenderTextureCmd.DestroyRenderTexture(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianError("RenderTexture Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple RenderTexture Dispose");
            }
        }

        public void Resize(uint a_width, uint a_height)
        {
            RenderTextureCmd.Resize(m_bufferAddr, a_width, a_height);
        }

        ~RenderTexture()
        {
            Dispose(false);
        }
    }
}