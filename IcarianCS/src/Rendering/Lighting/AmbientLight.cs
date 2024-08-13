// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineAmbientLightInteropStructures.h"

namespace IcarianEngine.Rendering.Lighting
{
    public class AmbientLight : Light, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static AmbientLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, AmbientLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Determines if the AmbientLight has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Lighting.LightType" /> of the AmbientLight
        /// </summary>
        public override LightType LightType
        {
            get 
            {
                return LightType.Ambient;
            }
        }

        /// <summary>
        /// The Definition used to create the AmbientLight
        /// </summary>
        public AmbientLightDef AmbientLightDef
        {
            get
            {
                return LightDef as AmbientLightDef;
            }
        }

        /// <summary>
        /// Render layer the AmbientLight is on
        /// </summary>
        /// Bitmask of render layers
        public override uint RenderLayer
        {
            get
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.RenderLayer;
            }
            set
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Color of the AmbientLight
        /// </summary>
        public override Color Color
        {
            get
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Color.ToColor();
            }
            set
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = value.ToVector4();

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Intensity of the AmbientLight
        /// </summary>
        public override float Intensity
        {
            get
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Intensity;
            }
            set
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(value, 0.0f);
                if (buffer.Intensity != v)
                {
                    buffer.Intensity = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Called when the light is initialized
        /// </summary>
        public override void Init()
        {
            base.Init();

            AmbientLightDef def = AmbientLightDef;

            m_bufferAddr = GenerateBuffer();

            if (def != null)
            {
                AmbientLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = def.Color.ToVector4();
                buffer.Intensity = def.Intensity;
                buffer.RenderLayer = def.RenderLayer;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Disposes of the object
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the object is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if the AmibentLight is being Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    DestroyBuffer(m_bufferAddr);    
                }
                else
                {
                    Logger.IcarianWarning("AmbientLight Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple AmbientLight Dispose");
            }
        }
        ~AmbientLight()
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