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
        /// Returns true if the object has been disposed
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// Returns the type of light
        /// </summary>
        public override LightType LightType
        {
            get 
            {
                return LightType.Ambient;
            }
        }

        /// <summary>
        /// Returns the light definition used to create the light
        /// </summary>
        public AmbientLightDef AmbientLightDef
        {
            get
            {
                return LightDef as AmbientLightDef;
            }
        }

        /// <summary>
        /// Render layer the ambient light is on
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
        /// Color of the ambient light
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
        /// Intensity of the ambient light
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

                buffer.Intensity = value;

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// Returns the shadow maps used by the light
        /// </summary>
        /// Unused for ambient lights
        public override IEnumerable<IRenderTexture> ShadowMaps
        {
            get
            {
                return null;
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

        ~AmbientLight()
        {
            Dispose(false);
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
        /// Called when the object is disposed
        /// </summary>
        /// <param name="a_disposing">True if the object is being disposed, false if it is being finalized</param>
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
    }
}