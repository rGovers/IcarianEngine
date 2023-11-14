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
    public class SpotLight : Light, IDestroy
    {
        static ConcurrentDictionary<uint, SpotLight> s_lightMap = new ConcurrentDictionary<uint, SpotLight>();

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

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Determines if the SpotLight has been disposed of.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// Returns the LightType of the SpotLight.
        /// </summary>
        public override LightType LightType
        {
            get
            {
                return LightType.Spot;
            }
        }

        /// <summary>
        /// Returns the Definition used to create the SpotLight.
        /// </summary>
        public SpotLightDef SpotLightDef
        {
            get
            {
                return Def as SpotLightDef;
            }
        }

        /// <summary>
        /// Returns the RenderLayer of the SpotLight.
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
        /// Returns the Color of the SpotLight.
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

                buffer.Intensity = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Returns the Position of the SpotLight.
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
        /// Returns inner cutoff angle of the SpotLight.
        /// </summary>
        /// Radians.
        public float InnerCutoffAngle
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return (float)Math.Acos(buffer.CutoffAngle.X);
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.CutoffAngle.X = (float)Math.Cos(value);

                SetBuffer(m_bufferAddr, buffer);
            }
        }
        /// <summary>
        /// Returns outer cutoff angle of the SpotLight.
        /// </summary>
        /// Radians.
        public float OuterCutoffAngle
        {
            get
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                return (float)Math.Acos(buffer.CutoffAngle.Y);
            }
            set
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.CutoffAngle.Y = (float)Math.Cos(value);

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Returns the Radius of the SpotLight.
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

                buffer.Radius = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Returns the ShadowMap of the SpotLight.
        /// </summary>
        public override IEnumerable<IRenderTexture> ShadowMaps
        {
            get
            {
                return null;
            }
        }

        /// <summary>
        /// ShadowMap of the SpotLight.
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
        /// Called when the SpotLight is created.
        /// </summary>
        public override void Init()
        {
            base.Init();

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            SpotLightDef spotDef = SpotLightDef;
            if (spotDef != null)
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = spotDef.RenderLayer;
                buffer.Color = spotDef.Color.ToVector4();
                buffer.Intensity = spotDef.Intensity;
                buffer.CutoffAngle = new Vector2(Mathf.Cos(spotDef.InnerCutoffAngle), Mathf.Cos(spotDef.OuterCutoffAngle));
                buffer.Radius = spotDef.Radius;

                SetBuffer(m_bufferAddr, buffer);
            }
            else
            {
                LightDef lightDef = LightDef;
                if (lightDef != null)
                {
                    SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                    buffer.RenderLayer = lightDef.RenderLayer;
                    buffer.Color = lightDef.Color.ToVector4();
                    buffer.Intensity = lightDef.Intensity;
                    buffer.CutoffAngle = new Vector2(Mathf.Cos(1.0f), Mathf.Cos(1.5f));
                    buffer.Radius = 10.0f;

                    SetBuffer(m_bufferAddr, buffer);
                }
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
        /// Disposes of the SpotLight.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the SpotLight is destroyed.
        /// </summary>
        /// <param name="a_disposing">Determines if the SpotLight is being disposed of.</param>
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