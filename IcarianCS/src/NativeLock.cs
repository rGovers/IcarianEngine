// Icarian Engine - C# Game Engine
// 
// License at end of file.

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