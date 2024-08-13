// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#ifdef ENABLE_EXPERIMENTAL

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

#endif

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