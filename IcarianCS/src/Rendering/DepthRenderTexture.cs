// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

#ifdef ENABLE_STACKTRACE
using System.Diagnostics;
#endif

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

        uint       m_bufferAddr = uint.MaxValue;

#ifdef ENABLE_STACKTRACE
        StackTrace m_stackTrace;
#endif

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
        public DepthRenderTexture(uint a_width, uint a_height)
        {
            m_bufferAddr = GenerateRenderTexture(a_width, a_height);

            s_bufferLookup.TryAdd(m_bufferAddr, this);

#ifdef ENABLE_STACKTRACE
            m_stackTrace = new StackTrace(true);
#endif
        }

        internal static DepthRenderTexture GetDepthRenderTexture(uint a_addr)
        {
            DepthRenderTexture buffer = null;
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
            if (a_width <= 0)
            {
                Logger.IcarianError($"DepthRenderTexture resize invalid width: {a_width}");

                return;
            }
            if (a_height <= 0)
            {
                Logger.IcarianError($"DepthRenderTexture resize invalid height: {a_height}");

                return;
            }

            Resize(m_bufferAddr, a_width, a_height);
        }

        /// <summary>
        /// Disposes the Depth Render Texture
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the DepthRenderTexture is Disposed/Finalized
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
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
                    Logger.IcarianWarning("DepthRenderTexture not Disposed");

#ifdef ENABLE_STACKTRACE
                    CallStack.PrintStackTrace(m_stackTrace);
#endif
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("DepthRenderTexture already Disposed");
            }
        }
        ~DepthRenderTexture()
        {
            Dispose(false);
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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