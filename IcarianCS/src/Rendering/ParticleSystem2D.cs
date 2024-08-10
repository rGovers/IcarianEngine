using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class ParticleSystem2D : ParticleSystem
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateComputeParticleSystem(uint a_particleBufferAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyComputeParticleSystem(uint a_bufferAddr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateGraphicsParticleSystem(uint a_computeBufferAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyGraphicsParticleSystem(uint a_bufferAddr);

        uint m_particleSystemAddr = uint.MaxValue;
        uint m_graphicsSystemAddr = uint.MaxValue;

        /// <summary>
        /// The ParticleSystem2DDef used to create this ParticleSystem2D
        /// </summary> 
        public ParticleSystem2DDef ParticleSystem2DDef
        {
            get
            {
                return Def as ParticleSystem2DDef;
            }
        }

        /// <summary>
        /// Called when the ParticleSystem2D is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            ComputeParticleBuffer buffer = GetComputeBuffer(ParticleBufferAddr);

            buffer.DisplayMode = ParticleDisplayMode.Quad;

            SetComputeBuffer(ParticleBufferAddr, buffer);

            m_particleSystemAddr = GenerateComputeParticleSystem(ParticleBufferAddr);
            m_graphicsSystemAddr = GenerateGraphicsParticleSystem(ParticleBufferAddr);
        }

        /// <summary>
        /// Called when the ParticleSystem2D is disposed
        /// </summary>
        /// <param name="a_disposing">Wheter the ParticleSystem2D is being disposed</param>
        protected override void Dispose(bool a_disposing)
        {
            if (!IsDisposed)
            {
                if (a_disposing)
                {
                    DestroyComputeParticleSystem(m_particleSystemAddr);
                    DestroyGraphicsParticleSystem(m_graphicsSystemAddr);
                }
                else
                {
                    Logger.IcarianWarning("ParticleSystem2D Failed to Dispose");
                }

                m_particleSystemAddr = uint.MaxValue;
                m_graphicsSystemAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple ParticleSystem2D Dispose");
            }

            base.Dispose(a_disposing);
        }
    }
}