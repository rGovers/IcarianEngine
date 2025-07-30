// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;

#ifdef ENABLE_STACKTRACE
using System.Diagnostics;
#endif

namespace IcarianEngine.Rendering.Shaders
{
    public enum ComputeMode
    {
        Graphics = 0,
        Compute = 1,
    };

    public class ComputeShader : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateGraphicsFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateComputeFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyGraphicsShader(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyComputeShader(uint a_addr);

        /// <summary>
        /// Adds a import target to the ComputeShader import table
        /// </summary>
        /// <param name="a_key">The import target to add</param>
        /// <param name="a_value">The import value to addd</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void AddImport(string a_key, string a_value);

        uint        m_internalAddr;
        ComputeMode m_computeMode;

#ifdef ENABLE_STACKTRACE
        StackTrace  m_stackTrace;
#endif

        /// <summary>
        /// Whether the ComputeShader has been Disposed
        /// </summary>
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

        /// <summary>
        /// Gets the compute pipeline the ComputeShader is on
        /// </summary>
        public ComputeMode ComputeMode
        {
            get
            {
                return m_computeMode;
            }
        }

        ComputeShader(uint a_addr, ComputeMode a_computeMode)
        {
            m_internalAddr = a_addr;
            m_computeMode = a_computeMode;

#ifdef ENABLE_STACKTRACE
            m_stackTrace = new StackTrace(true);
#endif
        }

        /// <summary>
        /// Loads a ComputeShader from a file
        /// </summary>
        /// <param name="a_path">The path to the ComputeShader</param>
        /// <param name="a_computeMode">The compute pipeline to put the ComputeShader on</param>
        /// <returns>The ComputeShader. Null on failure</returns>
        /// Supported formats:
        ///     .fcomp
        public static ComputeShader LoadComputeShader(string a_path, ComputeMode a_computeMode = ComputeMode.Graphics)
        {
            if (a_computeMode != ComputeMode.Graphics)
            {
                Logger.IcarianWarning($"Compute mode not implemented in LoadComputeShader: {a_computeMode}");

                return null;
            }

            uint addr = uint.MaxValue;

            switch (a_computeMode)
            {
            case ComputeMode.Graphics:
            {
                addr = GenerateGraphicsFromFile(a_path);

                break;
            }
            case ComputeMode.Compute:
            {
                addr = GenerateComputeFromFile(a_path);

                break;
            }
            }

            if (addr == uint.MaxValue)
            {
                return null;
            }

            return new ComputeShader(addr, a_computeMode);
        }

        /// <summary>
        /// Disposes of the ComputeShader
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the ComputeShader is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_internalAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    switch (m_computeMode)
                    {
                    case ComputeMode.Graphics:
                    {
                        DestroyGraphicsShader(m_internalAddr);

                        break;
                    }
                    case ComputeMode.Compute:
                    {
                        DestroyComputeShader(m_internalAddr);

                        break;
                    }
                    }
                }
                else
                {
                    Logger.IcarianError("ComputeShader not Disposed");

#ifdef ENABLE_STACKTRACE
                    CallStack.PrintStackTrace(m_stackTrace);
#endif  
                }

                m_internalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("ComputeShader already Disposed");
            }
        }
        ~ComputeShader()
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