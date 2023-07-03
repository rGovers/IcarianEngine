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

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        uint m_bufferAddr = uint.MaxValue;

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

        public CanvasRenderer()
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