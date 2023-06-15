using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine
{
    public class NativeLock : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateLock();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyLock(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SReadLock(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SReadUnlock(uint a_addr);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SWriteLock(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SWriteUnlock(uint a_addr);

        uint m_addr = uint.MaxValue;

        public bool IsDisposed
        {
            get
            {
                return m_addr == uint.MaxValue;
            }
        }

        public NativeLock()
        {
            m_addr = GenerateLock();
        }

        public void ReadLock()
        {
            SReadLock(m_addr);
        }
        public void ReadUnlock()
        {
            SReadUnlock(m_addr);
        }

        public void WriteLock()
        {
            SWriteLock(m_addr);
        }
        public void WriteUnlock()
        {
            SWriteUnlock(m_addr);
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_addr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyLock(m_addr);
                }
                else
                {
                    Logger.IcarianWarning("NativeLock Failed to Dispose");
                }

                m_addr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("NativeLock Multiple Dispose");
            }
        }

        ~NativeLock()
        {
            Dispose(false);
        }
    }
}