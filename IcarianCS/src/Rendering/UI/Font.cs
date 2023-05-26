using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering.UI
{
    public class Font : IDestroy
    {
        static Dictionary<uint, Font> s_bufferLookup = new Dictionary<uint, Font>();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFont(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint DestroyFont(uint a_addr);

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

        Font(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

            s_bufferLookup.Add(m_bufferAddr, this);
        }

        public static Font LoadFont(string a_path)
        {
            return new Font(GenerateFont(a_path));
        }

        internal static Font GetFont(uint a_buffer)
        {
            if (s_bufferLookup.ContainsKey(a_buffer))
            {
                return s_bufferLookup[a_buffer];
            }

            return null;
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyFont(m_bufferAddr);

                    s_bufferLookup.Remove(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Font Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Font Dispose");
            }
        }

        ~Font()
        {
            Dispose(false);
        }
    }
}