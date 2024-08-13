// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineDirectionalLightInteropStructures.h"

namespace IcarianEngine.Rendering.Lighting
{
    public class DirectionalLight : ShadowLight, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static DirectionalLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, DirectionalLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddShadowMap(uint a_addr, uint a_shadowMapAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void RemoveShadowMap(uint a_addr, uint a_shadowMapAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint[] GetShadowMaps(uint a_addr);

        static ConcurrentDictionary<uint, DirectionalLight> s_lightMap = new ConcurrentDictionary<uint, DirectionalLight>();

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Determines if the DirectionalLight has been Disposed/Finalised
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
        /// The <see cref="IcarianEngine.Rendering.Lighting.LightType" /> of the DirectionalLight
        /// </summary>
        public override LightType LightType
        {
            get
            {
                return LightType.Directional;
            }
        }

        /// <summary>
        /// The Definition used to create the DirectionalLight
        /// </summary>
        public DirectionalLightDef DirectionalLightDef
        {
            get
            {
                return Def as DirectionalLightDef;
            }
        }

        /// <summary>
        /// Render layers the DirectionalLight is a part of
        /// </summary>
        /// Bitmask of render layers.
        public override uint RenderLayer
        {
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.RenderLayer;
            }
            set
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Color of the DirectionalLight
        /// </summary>
        public override Color Color
        {
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Color.ToColor();
            }
            set
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = value.ToVector4();

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Intensity of the DirectionalLight
        /// </summary>
        public override float Intensity
        {
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Intensity;
            }
            set
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(value, 0.0f);
                if (v != buffer.Intensity)
                {
                    buffer.Intensity = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Shadow Bias used by the DirectionalLight
        /// </summary>
        public override Vector2 ShadowBias 
        { 
            get
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.ShadowBias;
            } 
            set
            { 
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                if (buffer.ShadowBias != value)
                {
                    buffer.ShadowBias = value;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Shadow Maps used by the DirectionalLight
        /// </summary>
        public override IEnumerable<IRenderTexture> ShadowMaps
        {
            get
            {
                uint[] shadowMapAddrs = GetShadowMaps(m_bufferAddr);
                foreach (uint shadowMapAddr in shadowMapAddrs)
                {
                    DepthRenderTexture shadowMap = DepthRenderTexture.GetDepthRenderTexture(shadowMapAddr);
                    if (shadowMap != null)
                    {
                        yield return shadowMap;
                    }
                }
            }
        }

        /// <summary>
        /// Called when the DirectionalLight is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            LightDef def = LightDef;
            if (def != null)
            {
                DirectionalLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = def.RenderLayer;
                buffer.Color = def.Color.ToVector4();
                buffer.Intensity = def.Intensity;
                
                ShadowLightDef shadowDef = ShadowLightDef;
                if (shadowDef != null)
                {
                    buffer.ShadowBias = new Vector2(shadowDef.ShadowBiasConstant, shadowDef.ShadowBiasSlope);
                }

                SetBuffer(m_bufferAddr, buffer);
            }

            s_lightMap.TryAdd(m_bufferAddr, this);
        }

        /// <summary>
        /// Added a ShadowMap to the DirectionalLight
        /// </summary>
        /// <param name="a_shadowMap">ShadowMap to add.</param>
        /// Refer to the cascades of the RenderPipeline for limitations on the number of ShadowMaps
        /// Default is 6 cascades.
        public void AddShadowMap(DepthRenderTexture a_shadowMap)
        {
            if (a_shadowMap != null)
            {
                AddShadowMap(m_bufferAddr, a_shadowMap.BufferAddr);
            }
            else
            {
                Logger.IcarianError("DirectionalLight AddShadowMap null DepthRenderTexture");
            }
        }
        /// <summary>
        /// Removes a ShadowMap from the DirectionalLight
        /// </summary>
        /// <param name="a_shadowMap">ShadowMap to remove</param>
        public void RemoveShadowMap(DepthRenderTexture a_shadowMap)
        {
            if (a_shadowMap != null)
            {
                RemoveShadowMap(m_bufferAddr, a_shadowMap.BufferAddr);
            }
            else
            {
                Logger.IcarianError("DirectionalLight RemoveShadowMap null DepthRenderTexture");
            }
        }

        internal static DirectionalLight GetLight(uint a_addr)
        {
            DirectionalLight light = null;

            s_lightMap.TryGetValue(a_addr, out light);

            return light;
        }

        /// <summary>
        /// Disposes of the DirectionalLight
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the DirectionalLight is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if the DirectionalLight is being Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyBuffer(m_bufferAddr);

                    s_lightMap.TryRemove(m_bufferAddr, out DirectionalLight _);
                }
                else
                {
                    Logger.IcarianWarning("DirectionalLight Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple DirectionalLight Dispose");
            }
        }
        ~DirectionalLight()
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