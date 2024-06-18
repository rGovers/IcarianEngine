using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EnginePointLightInteropStructures.h"

namespace IcarianEngine.Rendering.Lighting
{
    public class PointLight : ShadowLight, IDestroy
    {
        static ConcurrentDictionary<uint, PointLight> s_lightMap = new ConcurrentDictionary<uint, PointLight>();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static PointLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, PointLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetShadowMap(uint a_addr, uint a_shadowMapAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GetShadowMap(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Determines if the PointLight has been disposed of.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// Returns the LightType of the PointLight.
        /// </summary>
        public override LightType LightType
        {
            get
            {
                return LightType.Point;
            }
        }

        /// <summary>
        /// Returns the Definition used to create the PointLight.
        /// </summary>
        public PointLightDef PointLightDef
        {
            get
            {
                return Def as PointLightDef;
            }
        }

        /// <summary>
        /// Render layers the PointLight is a part of.
        /// </summary>
        /// Bitmask of RenderLayers.
        public override uint RenderLayer 
        { 
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.RenderLayer;   
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = value;

                SetBuffer(m_bufferAddr, buffer);
            } 
        }

        /// <summary>
        /// Color of the PointLight.
        /// </summary>
        public override Color Color 
        {
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Color.ToColor();
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.Color = value.ToVector4();

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Intensity of the PointLight.
        /// </summary>
        public override float Intensity 
        {
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);
                
                return buffer.Intensity;
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(0.0f, value);

                if (buffer.Intensity != v)
                {   
                    buffer.Intensity = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Radius of the PointLight.
        /// </summary>
        public float Radius
        {
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.Radius;
            }
            set
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                float v = Mathf.Max(0.0f, value);

                if (buffer.Radius != v)
                {
                    buffer.Radius = v;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Shadow Bias used by the PointLight
        /// </summary>
        public override Vector2 ShadowBias 
        { 
            get
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                return buffer.ShadowBias;
            } 
            set
            { 
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                if (buffer.ShadowBias != value)
                {
                    buffer.ShadowBias = value;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Returns the Light Shadow Maps
        /// </summary>
        public override IEnumerable<IRenderTexture> ShadowMaps
        {
            get
            {
                DepthCubeRenderTexture shadowMap = ShadowMap;
                if (shadowMap != null)
                {
                    yield return shadowMap;
                }
            }
        }

        /// <summary>
        /// Returns the CubeMap used for the PointLight ShadowMap.
        /// </summary>
        public DepthCubeRenderTexture ShadowMap
        {
            get
            {
                uint shadowMapAddr = GetShadowMap(m_bufferAddr);

                if (shadowMapAddr == uint.MaxValue)
                {
                    return null;
                }

                return DepthCubeRenderTexture.GetDepthCubeRenderTexture(shadowMapAddr);
            }
            set
            {
                if (value == null)
                {
                    SetShadowMap(m_bufferAddr, uint.MaxValue);
                }
                else
                {
                    SetShadowMap(m_bufferAddr, value.BufferAddr);
                }
            }
        }

        /// <summary>
        /// Called when the PointLight is created.
        /// </summary>
        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            PointLightDef pointDef = PointLightDef;
            if (pointDef != null)
            {
                PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = pointDef.RenderLayer;
                buffer.Color = pointDef.Color.ToVector4();
                buffer.Intensity = pointDef.Intensity;
                buffer.Radius = pointDef.Radius;

                SetBuffer(m_bufferAddr, buffer);
            }
            else
            {
                LightDef lightDef = LightDef;
                if (lightDef != null)
                {
                    PointLightBuffer buffer = GetBuffer(m_bufferAddr);

                    buffer.RenderLayer = lightDef.RenderLayer;
                    buffer.Color = lightDef.Color.ToVector4();
                    buffer.Intensity = lightDef.Intensity;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }

            s_lightMap.TryAdd(m_bufferAddr, this);
        }

        internal static PointLight GetLight(uint a_addr)
        {
            if (s_lightMap.TryGetValue(a_addr, out PointLight light))
            {
                return light;
            }

            return null;
        }

        ~PointLight()
        {
            Dispose(false);
        }
        /// <summary>
        /// Disposes of the PointLight.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the PointLight is disposed of.
        /// </summary>
        /// <param name="a_disposing">Determines if the PointLight is being disposed of.</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyBuffer(m_bufferAddr);

                    s_lightMap.TryRemove(m_bufferAddr, out PointLight _);
                }
                else
                {
                    Logger.IcarianWarning("PointLight Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple PointLight Dispose");
            }
        }
    }
}