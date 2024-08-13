// Icarian Engine - C# Game Engine
// 
// License at end of file.

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