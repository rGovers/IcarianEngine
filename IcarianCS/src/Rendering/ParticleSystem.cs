// Icarian Engine - C# Game Engine
// 
// License at end of file.

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
        extern static uint GenerateComputeBuffer(uint a_transformAddr);
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
        /// The Render Layer for the particle system
        /// </summary>
        public uint RenderLayer
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.RenderLayer;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.RenderLayer != value)
                {
                    buffer.RenderLayer = value;

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
        /// The gravity to apply to particles
        /// </summary>
        public Vector3 Gravity
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.Gravity;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.Gravity != value)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.Gravity = value;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The lifetime of particles
        /// </summary>
        public float Lifetime
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.Lifetime;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.Lifetime != value)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.Lifetime = value;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The size of the particle at the start of its lifetime
        /// </summary>
        public float StartSize
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.StartSize;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.StartSize != value)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.StartSize = value;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The size of the particle at the end of its lifetime
        /// </summary>
        public float EndSize
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.EndSize;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.EndSize != value)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.EndSize = value;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The scale of emitter velocity
        /// </summary>
        public float EmitterVelocityScale
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.EmitterVelocityScale;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.EmitterVelocityScale != value)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.EmitterVelocityScale = value;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The initial velocity to apply to particles
        /// </summary>
        public Vector3 InitialVelocity
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.InitialVelocity;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                if (buffer.InitialVelocity != value)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.InitialVelocity = value;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// The color of the particle at the start of its lifetime
        /// </summary>
        public Color StartColor
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.StartColour.ToColor();
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                Vector4 colour = value.ToVector4();

                if (buffer.StartColour != colour)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.StartColour = colour;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }
        /// <summary>
        /// The color of the particle at the end of its lifetime
        /// </summary>
        public Color EndColor
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return buffer.EndColour.ToColor();
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                Vector4 colour = value.ToVector4();

                if (buffer.EndColour != colour)
                {
                    unchecked
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.RefreshBit);
                    }

                    buffer.EndColour = colour;

                    SetComputeBuffer(m_particleBufferAddr, buffer);
                }
            }
        }

        /// <summary>
        /// Determine if the particle system is in burst mode
        /// </summary>
        public bool IsBurst
        {
            get
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                return (buffer.Flags & (int)ComputeParticleBuffer.BurstBit) != 0;
            }
            set
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                bool curValue = (buffer.Flags & 0b1 << (int)ComputeParticleBuffer.BurstBit) != 0;

                if (curValue != value)
                {
                    unchecked
                    {
                        if (value)
                        {
                            buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.BurstBit);
                        }
                        else
                        {
                            buffer.Flags &= (byte)~(0b1 << (int)ComputeParticleBuffer.BurstBit);
                        }

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

            m_particleBufferAddr = GenerateComputeBuffer(Transform.InternalAddr);

            ParticleSystemDef def = ParticleSystemDef;
            if (def != null)
            {
                ComputeParticleBuffer buffer = GetComputeBuffer(m_particleBufferAddr);

                buffer.MaxParticles = def.MaxParticles;
                buffer.EmitterRatio = def.EmitterRatio;
                buffer.RenderLayer = def.RenderLayer;
                buffer.EmitterType = def.EmitterType;
                buffer.EmitterRadius = def.EmitterRadius;
                buffer.Gravity = def.Gravity;
                buffer.InitialVelocity = def.InitialVelocity;
                buffer.StartColour = def.StartColor.ToVector4();
                buffer.EndColour = def.EndColor.ToVector4();
                buffer.StartSize = def.StartSize;
                buffer.EndSize = def.EndSize;
                buffer.Lifetime = def.Lifetime;
                buffer.EmitterVelocityScale = def.EmitterVelocityScale;
                buffer.Flags = 0;

                unchecked
                {
                    if (def.Burst)
                    {
                        buffer.Flags |= (byte)(0b1 << (int)ComputeParticleBuffer.BurstBit);
                    }

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