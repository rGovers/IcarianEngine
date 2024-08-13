// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;

#ifdef ENABLE_EXPERIMENTAL

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