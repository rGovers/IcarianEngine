// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

#include "EngineLightInteropStructures.h"

namespace IcarianEngine.Rendering.Lighting
{
    public abstract class Light : Component
    {
        /// <summary>
        /// Returns the LightType of the Light.
        /// </summary>
        public abstract LightType LightType
        {
            get;
        }

        /// <summary>
        /// Returns the RenderLayer of the Light.
        /// </summary>
        /// Bitmask of the layers the light will affect.
        public abstract uint RenderLayer
        {
            get;
            set;
        }

        /// <summary>
        /// Color of the Light.
        /// </summary>
        public abstract Color Color
        {
            get;
            set;
        }
        /// <summary>
        /// Intensity of the Light.
        /// </summary>
        public abstract float Intensity
        {
            get;
            set;
        }

        /// <summary>
        /// Returns the Definition used to create the Light.
        /// </summary>
        public LightDef LightDef
        {
            get
            {
                return Def as LightDef;
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