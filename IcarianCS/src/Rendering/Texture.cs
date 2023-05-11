using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class Texture : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path);
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

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        Texture(uint a_addr)
        {
            m_bufferAddr = a_addr;
        }

        public static Texture LoadTexture(string a_path)
        {
            uint addr = GenerateFromFile(a_path);
            if (addr != uint.MaxValue)
            {
                return new Texture(addr);
            }

            Logger.IcarianError($"Texture failed to load: {a_path}");

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
                    DestroyTexture(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Texture Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Texture Dispose");
            }
        }

        ~Texture()
        {
            Dispose(false);
        }
    }
}