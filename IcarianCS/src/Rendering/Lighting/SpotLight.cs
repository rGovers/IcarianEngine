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

#include "EngineSpotLightInteropStructures.h"

namespace IcarianEngine.Rendering.Lighting
{
    public class SpotLight : ShadowLight, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static SpotLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, SpotLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetShadowMap(uint a_addr, uint a_shadowMapAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetShadowMap(uint a_addr);

        static ConcurrentDictionary<uint, SpotLight> s_lightMap = new ConcurrentDictionary<uint, SpotLight>();

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Determines if the SpotLight has been Disposed/Finalised
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
        /// The <see cref="IcarianEngine.Rendering.Lighting.LightType" /> of the SpotLight
        /// </summary>
        public override LightType LightType
        {
            get
            {
                return LightType.Spot;
            }
        }

        /// <summary>
        /// The Definition used to create the SpotLight
        /// </summary>
        public SpotLightDef SpotLightDef
        {
            get
            {
                return Def as SpotLightDef;
            }
        }

        /// <summary>
        /// The RenderLayer of the SpotLight
        /// </summary>
        /// Bitmask of RenderLayers to render the SpotLight on.
        public override uint RenderLayer 
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.RenderLayer;
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// The Color of the SpotLight
        /// </summary>
        public override Color Color 
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Color.ToColor();
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = value.ToVector4();

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Returns the Intensity of the SpotLight.
        /// </summary>
        public override float Intensity 
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Intensity;
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(value, 0.0f);
                if (v != buffer.Intensity)
                {
                    buffer.Intensity = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The cutoff of the SpotLight
        /// </summary>
        /// X is inner cutoff angle, Y is outer cutoff angle.
        /// Dot based on the direction of the SpotLight.
        public Vector2 Cutoff
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.CutoffAngle; 
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.CutoffAngle = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// The inner cutoff angle of the SpotLight
        /// </summary>
        /// Radians.
        public float InnerCutoffAngle
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return Mathf.Acos(buffer.CutoffAngle.X);
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(Mathf.Cos(value), 0.0f);
                if (v != buffer.CutoffAngle.X)
                {
                    buffer.CutoffAngle.X = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }
        /// <summary>
        /// The outer cutoff angle of the SpotLight
        /// </summary>
        /// Radians.
        public float OuterCutoffAngle
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return Mathf.Acos(buffer.CutoffAngle.Y);
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(Mathf.Cos(value), 0.0f);
                if (v != buffer.CutoffAngle.Y)
                {   
                    buffer.CutoffAngle.Y = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The Radius of the SpotLight.
        /// </summary>
        public float Radius
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Radius;
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(value, 0.0f);
                if (v != buffer.Radius)
                {
                    buffer.Radius = value;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Shadow Bias used by the SpotLight
        /// </summary>
        public override Vector2 ShadowBias 
        { 
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.ShadowBias;
            } 
            set
            { 
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                if (buffer.ShadowBias != value)
                {
                    buffer.ShadowBias = value;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The Shadow Map of the SpotLight
        /// </summary>
        public override IEnumerable<IRenderTexture> ShadowMaps
        {
            get
            {
                DepthRenderTexture shadowMap = ShadowMap;
                if (shadowMap != null)
                {
                    yield return shadowMap;
                }
            }
        }

        /// <summary>
        /// ShadowMap of the SpotLight
        /// </summary>
        public DepthRenderTexture ShadowMap
        {
            get
            {
                uint shadowMapAddr = GetShadowMap(m_bufferAddr);

                if (shadowMapAddr != uint.MaxValue)
                {
                    return DepthRenderTexture.GetDepthRenderTexture(shadowMapAddr);
                }

                return null;
            }
            set
            {
                if (value != null)
                {
                    SetShadowMap(m_bufferAddr, value.BufferAddr);
                }
                else
                {
                    SetShadowMap(m_bufferAddr, uint.MaxValue);
                }
            }
        }

        /// <summary>
        /// Called when the SpotLight is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            LightDef lightDef = LightDef;
            if (lightDef != null)
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = lightDef.RenderLayer;
                buffer.Color = lightDef.Color.ToVector4();
                buffer.Intensity = lightDef.Intensity;
                buffer.CutoffAngle = new Vector2(Mathf.Cos(1.0f), Mathf.Cos(1.5f));
                buffer.Radius = 10.0f;

                ShadowLightDef shadowDef = ShadowLightDef;
                if (shadowDef != null)
                {
                    buffer.ShadowBias = new Vector2(shadowDef.ShadowBiasConstant, shadowDef.ShadowBiasSlope);

                    SpotLightDef spotDef = SpotLightDef;
                    if (spotDef != null)
                    {
                        buffer.CutoffAngle = new Vector2(Mathf.Cos(spotDef.InnerCutoffAngle), Mathf.Cos(spotDef.OuterCutoffAngle));
                        buffer.Radius = spotDef.Radius;
                    }
                }

                SetBuffer(m_bufferAddr, buffer);
            }

            s_lightMap.TryAdd(m_bufferAddr, this);
        }

        internal static SpotLight GetLight(uint a_addr)
        {
            SpotLight light = null;
  
            s_lightMap.TryGetValue(a_addr, out light);

            return light;
        }

        ~SpotLight()
        {
            Dispose(false);
        }
        /// <summary>
        /// Disposes of the SpotLight
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the SpotLight is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if the SpotLight is being Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyBuffer(m_bufferAddr);

                    s_lightMap.TryRemove(m_bufferAddr, out SpotLight _);
                }
                else
                {
                    Logger.IcarianWarning("SpotLight Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple SpotLight Dispose");
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