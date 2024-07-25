using System;

namespace IcarianEngine.Rendering
{
    public class RenderTexture : IRenderTexture
    {
        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the RenderTexture has been disposed/finalised
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
        /// The width of the RenderTexture
        /// </summary>
        public uint Width
        {
            get
            {
                return RenderTextureCmd.GetWidth(m_bufferAddr);
            }
        }
        /// <summary>
        /// The height of the RenderTexture
        /// </summary>
        public uint Height
        {
            get
            {
                return RenderTextureCmd.GetHeight(m_bufferAddr);
            }
        }
        /// <summary>
        /// If the RenderTexture has a depth component
        /// </summary>
        public bool HasDepth
        {
            get
            {
                return RenderTextureCmd.HasDepth(m_bufferAddr) != 0;
            }
        }

        public RenderTexture(uint a_width, uint a_height, bool a_depth = false, bool a_hdr = false, uint a_channelCount = 4)
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

            m_bufferAddr = RenderTextureCmd.GenerateRenderTexture(1, a_width, a_height, depthVal, hdrVal, a_channelCount);

            RenderTextureCmd.PushRenderTexture(m_bufferAddr, this);
        }
        public RenderTexture(uint a_width, uint a_height, DepthRenderTexture a_depthTexture, bool a_hdr = false, uint a_channelCount = 4)
        {
            if (a_hdr)
            {
                m_bufferAddr = RenderTextureCmd.GenerateRenderTextureD(1, a_width, a_height, a_depthTexture.BufferAddr, 1, a_channelCount);
            }
            else
            {
                m_bufferAddr = RenderTextureCmd.GenerateRenderTextureD(1, a_width, a_height, a_depthTexture.BufferAddr, 0, a_channelCount);
            }

            RenderTextureCmd.PushRenderTexture(m_bufferAddr, this);
        }

        /// <summary>
        /// Resizes the RenderTexture
        /// </summary>
        /// <param name="a_width">The new width of the RenderTexture</param>
        /// <param name="a_height">The new height of the RenderTexture</param>
        public void Resize(uint a_width, uint a_height)
        {
            RenderTextureCmd.Resize(m_bufferAddr, a_width, a_height);
        }

        /// <summary>
        /// Disposes of the RenderTexture
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the RenderTexture is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether this has called from Dispose</param>
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
        ~RenderTexture()
        {
            Dispose(false);
        }
    }
}