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