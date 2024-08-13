// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Runtime.CompilerServices;

#include "EngineTextUIElementInterop.h"
#include "InteropBinding.h"

ENGINE_TEXTUIELEMENT_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Rendering.UI
{
    public class TextUIElement : UIElement
    {
        /// <summary>
        /// The text of the TextUIElement
        /// </summary>
        public string Text
        {
            get
            {
                return TextUIElementInterop.GetText(BufferAddr);
            }
            set
            {
                TextUIElementInterop.SetText(BufferAddr, value);
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.UI.Font" /> of the TextUIElement
        /// </summary>
        public Font Font
        {
            get
            {
                return Font.GetFont(TextUIElementInterop.GetFont(BufferAddr));
            }
            set
            {
                if (value != null)
                {
                    TextUIElementInterop.SetFont(BufferAddr, value.BufferAddr);
                }
                else
                {
                    TextUIElementInterop.SetFont(BufferAddr, uint.MaxValue);
                }
            }
        }

        /// <summary>
        /// The font size of the TextUIElement
        /// </summary>
        public float FontSize
        {
            get
            {
                return TextUIElementInterop.GetFontSize(BufferAddr);
            }
            set
            {
                TextUIElementInterop.SetFontSize(BufferAddr, value);
            }
        }

        public TextUIElement() : base(TextUIElementInterop.CreateTextElement())
        {
            
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