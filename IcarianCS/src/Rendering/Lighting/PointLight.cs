using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering.Lighting
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct PointLightBuffer
    {
        public uint TransformAddr;
        public uint RenderLayer;
        public Vector4 Color;
        public float Intensity;
        public float Radius;
        public IntPtr Data;
    }

    public class PointLight : Light, IDestroy
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

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        public override LightType LightType
        {
            get
            {
                return LightType.Point;
            }
        }

        public PointLightDef PointLightDef
        {
            get
            {
                return Def as PointLightDef;
            }
        }

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

                buffer.Intensity = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

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

                buffer.Radius = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

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

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

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