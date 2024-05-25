using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineParticleSystemInteropStructures.h"

namespace IcarianEngine.Rendering
{
    public abstract class ParticleSystem : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateComputeBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyComputeBuffer(uint a_bufferAddr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static ComputeParticleBuffer GetComputeBuffer(uint a_bufferAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void SetComputeBuffer(uint a_bufferAddr, ComputeParticleBuffer a_buffer);

        uint m_particleBufferAddr = uint.MaxValue;

        internal uint ParticleBufferAddr
        {
            get
            {
                return m_particleBufferAddr;
            }
        }

        /// <summary>
        /// Wheter the ParticleSystem has been disposed
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_particleBufferAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The ParticleSystemDef use to create this ParticleSystem
        /// </summary>
        public ParticleSystemDef ParticleSystemDef
        {
            get
            {
                return Def as ParticleSystemDef;
            }
        }

        /// <summary>
        /// Determines if the values of the Particle System are dynamic
        /// </summary>
        /// If the values are not dynamic requires regenerating the Particle System on change of values
        public bool IsDynamic
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return (buffer.Flags & 0b1 << (int)ComputeParticleBuffer.DynamicBit) != 0;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                bool curValue = (buffer.Flags & 0b1 << (int)ComputeParticleBuffer.DynamicBit) != 0;

                if (value != curValue)
                {
                    unchecked
                    {
                        if (value)
                        {
                            buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.DynamicBit);
                        }
                        else
                        {
                            buffer.Flags &= (byte)~(0b1 << (int)ComputeParticleBuffer.DynamicBit);
                        }

                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }   
            }
        }

        /// <summary>
        /// The emitter type of the Particle System
        /// </summary>
        public ParticleEmitterType EmitterType
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.EmitterType;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.EmitterType != value)
                {
                    buffer.EmitterType = value;

                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The maximum number of Particles a particle system can have
        /// </summary>
        public uint MaxParticles
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.MaxParticles;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.MaxParticles != value)
                {
                    buffer.MaxParticles = value;

                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.GraphicsRefreshBit);
                    }

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The odds of creating a particle
        /// </summary>
        public float EmitterRatio
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.EmitterRatio;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.EmitterRatio != value)
                {
                    buffer.EmitterRatio = value;

                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Called when the ParticleSystem is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            m_particleBufferAddr = GenerateComputeBuffer();

            ParticleSystemDef def = ParticleSystemDef;
            if (def != null)
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                buffer.EmitterType = def.EmitterType;
                buffer.EmitterRadius = def.EmitterRadius;
                buffer.Flags = 0;

                unchecked
                {
                    if (def.AutoPlay)
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.PlayBit);
                    }
                }
                
                SetComputeBuffer(m_particleBufferAddr, buffer);
            }
        }

        /// <summary>
        /// Start the particle simulation
        /// </summary>
        public void Play()
        {
            ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

            unchecked
            {
                buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.PlayBit);
            }

            SetComputeBuffer(m_particleBufferAddr, buffer);
        }

        /// <summary>
        /// Disposes the ParticleSystem2D
        /// <summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the ParticleSystem is disposed
        /// </summary>
        /// <param name="a_disposing">Wheter the ParticleSystem2D is being disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (!IsDisposed)
            {
                if (a_disposing)
                {
                    DestroyComputeBuffer(m_particleBufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("ParticleSystem Failed to Dispose");
                }

                m_particleBufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Mutiple ParticleSystem Dispose");
            }
        }
        ~ParticleSystem()
        {
            Dispose(false);
        }
    }
}