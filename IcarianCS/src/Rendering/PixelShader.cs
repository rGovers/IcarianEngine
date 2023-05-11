using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class PixelShader : IDestroy
    {
        uint m_internalAddr = uint.MaxValue;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyShader(uint a_addr);

        public bool IsDisposed
        {
            get
            {
                return m_internalAddr == uint.MaxValue;
            }
        }

        internal uint InternalAddr
        {
            get
            {
                return m_internalAddr;
            }
        }

        PixelShader(uint a_addr)
        {
            m_internalAddr = a_addr;
        }

        public static PixelShader LoadPixelShader(string a_path)
        {
            uint addr = GenerateFromFile(a_path);

            if (addr != uint.MaxValue)
            {
                return new PixelShader(addr);
            }
            else
            {
                Logger.IcarianError($"Failed to load PixelShader: {a_path}");
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
            if(m_internalAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyShader(m_internalAddr);
                }
                else
                {
                    Logger.IcarianWarning("PixelShader Failed to Dispose");
                }

                m_internalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple PixelShader Dispose");
            }
        }

        ~PixelShader()
        {
            Dispose(false);
        }
    }
}