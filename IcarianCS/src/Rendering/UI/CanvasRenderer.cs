// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering.UI
{
    public class CanvasRenderer : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetCanvas(uint a_addr, uint a_canvasAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetCanvas(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetRenderTexture(uint a_addr, uint a_renderTextureAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetRenderTexture(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        public Canvas Canvas
        {
            get
            {
                return Canvas.GetCanvas(GetCanvas(m_bufferAddr));
            }
            set
            {
                if (value != null)
                {
                    SetCanvas(m_bufferAddr, value.BufferAddr);
                }
                else
                {
                    SetCanvas(m_bufferAddr, uint.MaxValue);
                }
            }
        }

        public IRenderTexture RenderTexture
        {
            get
            {
                return RenderTextureCmd.GetRenderTexture(GetRenderTexture(m_bufferAddr));
            }
            set
            {
                SetRenderTexture(m_bufferAddr, RenderTextureCmd.GetTextureAddr(value));
            }
        }

        public override void Init()
        {
            m_bufferAddr = GenerateBuffer();
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
                    DestroyBuffer(m_bufferAddr);

                    m_bufferAddr = uint.MaxValue;
                }
                else
                {
                    Logger.IcarianWarning("CanvasRenderer Failed to Dispose");
                }
            }
            else
            {
                Logger.IcarianError("Multiple calls to Dispose on CanvasRenderer");
            }
        }

        ~CanvasRenderer()
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