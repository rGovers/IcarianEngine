// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class TextRendererDef : RendererDef
    {
        [EditorTooltip("The text for the text renderer")]
        public string Text;

        /// <summary>
        /// Path relative to the project for the <see cref="IcarianEngine.Rendering.Font" /> to load
        /// </summary>
        [EditorTooltip("Path relative to the project for the font to load"), EditorPathString(new string[] { ".ttf" })]
        public string FontPath;

        [EditorTooltip("The size of the font")]
        public float FontSize = 24.0f;

        [EditorTooltip("The amount to scale the text by")]
        public float TextScale = 1.0f;

        [EditorTooltip("The depth of the text")]
        public float TextDepth = 0.25f;

        public TextRendererDef()
        {
            ComponentType = typeof(TextRenderer);
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