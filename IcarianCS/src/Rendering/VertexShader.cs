using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class VertexShader : IDestroy
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

        VertexShader(uint a_addr)
        {   
            m_internalAddr = a_addr;
        }

        public static VertexShader LoadVertexShader(string a_path)
        {
            uint addr = GenerateFromFile(a_path);

            if (addr != uint.MaxValue)
            {
                return new VertexShader(addr);
            }
            else
            {
                Logger.IcarianError($"Failed to load VertexShader: {a_path}");
            }

            return null;
        }

        public override bool Equals(object a_obj)
        {
            if (a_obj == null && m_internalAddr == uint.MaxValue)
            {
                return true;
            }

            return base.Equals(a_obj);
        }
        public override int GetHashCode()
        {
            return base.GetHashCode();
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
                    Logger.IcarianWarning("VertexShader Failed to Dispose");
                }

                m_internalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("FlareCS: Multiple VertexShader Dispose");
            }
        }

        ~VertexShader()
        {
            Dispose(false);
        }
    }
}