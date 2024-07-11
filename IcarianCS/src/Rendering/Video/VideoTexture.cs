using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering.Video
{
    public class VideoTexture
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateTexture(uint a_clipAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyTexture(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        VideoTexture(uint a_addr)
        {
            m_bufferAddr = a_addr;
        }

        public static VideoTexture FromClip(VideoClip a_clip)
        {
            if (a_clip == null)
            {
                Logger.IcarianWarning("VideoTexture null VideoClip");

                return null;
            }

            uint addr = GenerateTexture(a_clip.InternalAddr);
            if (addr != uint.MaxValue)
            {
                return new VideoTexture(addr);
            }

            Logger.IcarianError("Failed to create VideoTexture");

            return null;
        }
    }
}