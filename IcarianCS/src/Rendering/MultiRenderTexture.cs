// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class MultiRenderTexture : IRenderTexture
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetTextureCount(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the MultiRenderTexture had been disposed/finalised
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
        /// The width of the MultiRenderTexture
        /// </summary>
        public uint Width
        {
            get
            {
                return RenderTextureCmd.GetWidth(m_bufferAddr);
            }
        }
        /// <summary>
        /// The height of the MultiRenderTexture
        /// </summary>
        public uint Height
        {
            get
            {
                return RenderTextureCmd.GetHeight(m_bufferAddr);
            }
        }

        /// <summary>
        /// Gets the texture count not including depth
        /// </summary>
        public uint TextureCount
        {
            get
            {
                return GetTextureCount(m_bufferAddr);
            }
        }

        /// <summary>
        /// If the MutliRenderTexture has a depth component
        /// </summary>
        public bool HasDepth
        {
            get
            {
                return RenderTextureCmd.HasDepth(m_bufferAddr) != 0;
            }
        }

        public MultiRenderTexture(uint a_count, uint a_width, uint a_height, bool a_depth = false, bool a_hdr = false, uint a_channelCount = 4)
        {
            uint hdrVal = 0;
            if (a_hdr)
            {
                hdrVal = 1;
            }
            
            uint depthVal = 0;
            if (a_depth)
            {
                depthVal = 1;
            }

            m_bufferAddr = RenderTextureCmd.GenerateRenderTexture(a_count, a_width, a_height, depthVal, hdrVal, a_channelCount);

            RenderTextureCmd.PushRenderTexture(m_bufferAddr, this);
        }
        public MultiRenderTexture(uint a_count, uint a_width, uint a_height, DepthRenderTexture a_depthTexture, bool a_hdr = false, uint a_channelCount = 4)
        {
            if (a_hdr)
            {
                m_bufferAddr = RenderTextureCmd.GenerateRenderTextureD(a_count, a_width, a_height, a_depthTexture.BufferAddr, 1, a_channelCount);
            }
            else
            {
                m_bufferAddr = RenderTextureCmd.GenerateRenderTextureD(a_count, a_width, a_height, a_depthTexture.BufferAddr, 0, a_channelCount);
            }

            RenderTextureCmd.PushRenderTexture(m_bufferAddr, this);
        }

        /// <summary>
        /// Resizes the MultiRenderTexture
        /// </summary>
        /// <param name="a_width">The new width of the RenderTexture</param>
        /// <param name="a_height">The new height of the RenderTexture</param>
        public void Resize(uint a_width, uint a_height)
        {
            RenderTextureCmd.Resize(m_bufferAddr, a_width, a_height);
        }

        /// <summary>
        /// Disposes of the MultiRenderTexture
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the MultiRenderTexture is being Disposed/Finalised
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
                    Logger.IcarianWarning("MultiRenderTexture Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple MultiRenderTexture Dispose");
            }
        }
        ~MultiRenderTexture()
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