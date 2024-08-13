// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEngine.Rendering.Lighting
{
    public abstract class ShadowLight : Light
    {
        /// <summary>
        /// The Definition used to create the ShadowLight
        /// </summary>
        public ShadowLightDef ShadowLightDef
        {
            get
            {
                return Def as ShadowLightDef;
            }
        }

        /// <summary>
        /// Shadow Map of the ShadowLight
        /// </summary>
        public abstract IEnumerable<IRenderTexture> ShadowMaps
        {
            get;
        }

        /// <summary>
        /// Shadow bias of the ShadowLight
        /// </summary>
        public abstract Vector2 ShadowBias
        {
            get;
            set;
        }

        /// <summary>
        /// The depth bias for the ShadowLight
        /// </summary>
        public float ShadowBiasConstant
        {
            get
            {
                return ShadowBias.X;
            }
            set
            {
                Vector2 val = ShadowBias;

                val.X = value;

                ShadowBias = val;
            }
        }

        /// <summary>
        /// The depth slope for the ShadowLight
        /// </summary>
        public float ShadowBiasSlope
        {
            get
            {
                return ShadowBias.Y;
            }
            set
            {
                Vector2 val = ShadowBias;

                val.Y = value;

                ShadowBias = val;
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