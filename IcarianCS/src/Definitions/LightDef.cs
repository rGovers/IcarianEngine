// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class LightDef : ComponentDef
    {
        /// <summary>
        /// Used to determine if it will be rendered by a camera in a matching layer
        /// </summary>
        [EditorBitfield, EditorTooltip("Used to determine if it will be rendered by a camera in a matching layer.")]
        public uint RenderLayer = 0b1;
        /// <summary>
        /// Used to determine light color
        /// </summary>
        [EditorTooltip("Used to determine light color")]
        public Color Color = Color.White;
        /// <summary>
        /// Used to determine light strength
        /// </summary>
        [EditorTooltip("Used to determine light strength")]
        public float Intensity = 10.0f;

        public LightDef()
        {
            ComponentType = typeof(Light);
        }

        /// <summary>
        /// Called after the Def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (!ComponentType.IsSubclassOf(typeof(Light)))
            {
                Logger.IcarianError($"LightDef {DefName} Invalid ComponentType: {ComponentType}");
            }
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