using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Lighting
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct DirectionalLightBuffer
    {
        public uint TransformAddr;
        public uint RenderLayer;
        public Vector4 Color;
        public float Intensity;
        public IntPtr Data;
    }

    public class DirectionalLight : Light, IDestroy
    {
        static ConcurrentDictionary<uint, DirectionalLight> s_lightMap = new ConcurrentDictionary<uint, DirectionalLight>();

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
                return LightType.Directional;
            }
        }

        public DirectionalLightDef DirectionalLightDef
        {
            get
            {
                return Def as DirectionalLightDef;
            }
        }

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

                buffer.Intensity = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public override DepthRenderTexture[] ShadowMaps
        {
            get
            {
                return null;
            }
        }

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

                SetBuffer(m_bufferAddr, buffer);
            }

            s_lightMap.TryAdd(m_bufferAddr, this);
        }

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

        ~DirectionalLight()
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
    }
}