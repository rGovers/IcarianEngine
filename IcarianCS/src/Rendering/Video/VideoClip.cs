using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "InteropBinding.h"
#include "EngineVideoClipInterop.h"

ENGINE_VIDEOCLIP_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.Video
{
    public class VideoClip : IDestroy
    {
        uint m_bufferAddr = uint.MaxValue;

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue; 
            }
        }

        VideoClip(uint a_addr)
        {
            m_bufferAddr = a_addr;
        }

        public static VideoClip LoadVideoClip(string a_path)
        {
            uint addr = VideoClipInterop.GenerateFromFile(a_path);
            if (addr != uint.MaxValue)
            {
                return new VideoClip(addr);
            }

            Logger.IcarianWarning($"VideoClip failed to load: {a_path}");

            return null;
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
                    VideoClipInterop.DestroyClip(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("VideoClip failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("VideoClip already Disposed");
            }
        }
        ~VideoClip()
        {
            Dispose(false);
        }
    }
}