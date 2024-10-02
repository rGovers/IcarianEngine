// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class VertexShader : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyShader(uint a_addr);
        
        /// <summary>
        /// Adds a import target to the VertexShader import table
        /// </summary>
        /// <param name="a_key">The import target to add</param>
        /// <param name="a_value">The import value to addd</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static void AddImport(string a_key, string a_value);

        uint m_internalAddr = uint.MaxValue;

        /// <summary>
        /// Whether the VertexShader has been Disposed/Finalised
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

        VertexShader(uint a_addr)
        {   
            m_internalAddr = a_addr;
        }

        /// <summary>
        /// Loads a VertexShader from a file
        /// </summary>
        /// <param name="a_path">The path to the VertexShader</param>
        /// Supported formats:
        ///     .fvert
        /// @see IcarianEngine.AssetLibrary.LoadVertexShader
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

        /// <summary>
        /// Disposes of the VertexShader
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the VertexShader is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether it is being called from Dispose</param>
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
                Logger.IcarianError("Multiple VertexShader Dispose");
            }
        }
        ~VertexShader()
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