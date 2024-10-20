// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

#include "EngineFontInterop.h"
#include "InteropBinding.h"

ENGINE_FONT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class Font : IDestroy
    {
        static Dictionary<uint, Font> s_bufferLookup = new Dictionary<uint, Font>();

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether or not the Font has been Disposed/Finalised
        /// </summary>
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

        /// <summary>
        /// Loads a Font from file
        /// </summary>
        /// <param name="a_path">The path to load the Font from</param>
        /// <returns>The Font</returns>
        public static Font LoadFont(string a_path)
        {
            return new Font(FontInterop.GenerateFont(a_path));
        }

        /// <summary>
        /// Creates a <see cref="IcarianEngine.Rendering.Model" /> from a string
        /// </summary>
        /// <param name="a_string">The input string to make a <see cref="IcarianEngine.Rendering.Model" /> of</param>
        /// <param name="a_fontSize">The size of the font</param>
        /// <param name="a_scale">Scale to apply to the <see cref="IcarianEngine.Rendering.Model" /></param>
        /// <param name="a_depth">The depth of the <see cref="IcarianEngine.Rendering.Model" /></param>
        /// <returns>The model. Null on failure</returns>
        public Model CreateModel(string a_string, float a_fontSize, float a_scale, float a_depth)
        {
            uint addr = FontInterop.GenerateModel(m_bufferAddr, a_string, a_fontSize, a_scale, a_depth);
            if (addr == uint.MaxValue)
            {
                Logger.IcarianWarning("Failed to create model from string");

                return null;
            }

            return new Model(addr);
        }

        internal static Font GetFont(uint a_buffer)
        {
            if (s_bufferLookup.ContainsKey(a_buffer))
            {
                return s_bufferLookup[a_buffer];
            }

            return null;
        }

        /// <summary>
        /// Disposes of the Font
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Font is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    FontInterop.DestroyFont(m_bufferAddr);

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