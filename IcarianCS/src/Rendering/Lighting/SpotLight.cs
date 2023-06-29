using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering.Lighting
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct SpotLightBuffer
    {
        public uint TransformAddr;
        public uint RenderLayer;
        public Vector4 Color;
        public float Intensity;
        public Vector2 CutoffAngle;
        public float Radius;
        public IntPtr Data;
    }

    public class SpotLight : Light, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static SpotLightBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, SpotLightBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);

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
                return LightType.Spot;
            }
        }

        public SpotLightDef SpotLightDef
        {
            get
            {
                return Def as SpotLightDef;
            }
        }

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

            SpotLightDef spotDef = SpotLightDef;
            if (spotDef != null)
            {
                SpotLightBuffer buffer = GetBuffer(m_bufferAddr);

                buffer.RenderLayer = spotDef.RenderLayer;
                buffer.Color = spotDef.Color.ToVector4();
                buffer.Intensity = spotDef.Intensity;
                buffer.CutoffAngle = new Vector2((float)Math.Cos(spotDef.InnerCutoffAngle), (float)Math.Cos(spotDef.OuterCutoffAngle));
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
                    buffer.CutoffAngle = new Vector2((float)Math.Cos(1.0f), (float)Math.Cos(1.5f));
                    buffer.Radius = 10.0f;

                    SetBuffer(m_bufferAddr, buffer);
                }
            }
        }

        ~SpotLight()
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