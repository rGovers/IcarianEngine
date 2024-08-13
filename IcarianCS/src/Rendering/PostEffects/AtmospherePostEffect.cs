// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering.PostEffects
{
    public class AtmospherePostEffect : PostEffect, IDestroy
    {
        [StructLayout(LayoutKind.Sequential)]
        struct ShaderData
        {
            public Vector4 SunDir;
            public Vector4 Rayleigh;
            public Vector4 Mie;
            public Vector4 SunColor;
            public Vector4 HazeColor;
        };

        VertexShader m_quadVertex;
        PixelShader  m_atmospherePixel;

        Material     m_material;

        ShaderData   m_data;

        /// <summary>
        /// Whether or not the AtmospherePostEffect has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_material == null;
            }
        }

        /// <summary>
        /// The direction from origin to the sun
        /// </summary>
        public Vector3 SunDirection
        {
            get
            {
                return m_data.SunDir.XYZ;
            }
            set
            {
                Vector3 v = Vector3.Normalized(value);

                if (m_data.SunDir.XYZ != v)
                {
                    m_data.SunDir.XYZ = v;

                    m_material.SetUserUniform(m_data);
                }
            }
        }

        /// <summary>
        /// The Rayleigh color as a Vector
        /// <summary>
        public Vector3 RayleighColorVector
        {
            get
            {
                return m_data.Rayleigh.XYZ;
            }
            set
            {
                if (m_data.Rayleigh.XYZ != value)
                {
                    m_data.Rayleigh.XYZ = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }

        /// <summary>
        /// The G value of Mie
        /// </summary>
        public float MieG
        {
            get
            {
                return m_data.Mie.W;
            }
            set
            {
                if (m_data.Mie.W != value)
                {
                    m_data.Mie.W = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }
        /// <summary>
        /// The Mie color as a Vector
        /// </summary>
        public Vector3 MieColorVector
        {
            get
            {
                return m_data.Mie.XYZ;
            }
            set
            {
                if (m_data.Mie.XYZ != value)
                {
                    m_data.Mie.XYZ = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }

        /// <summary>
        /// The color of the sun as a Vector
        /// <summary>
        public Vector3 SunColorVector
        {
            get
            {
                return m_data.SunColor.XYZ;
            }
            set
            {
                if (m_data.SunColor.XYZ != value)
                {
                    m_data.SunColor.XYZ = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }
        /// <summary>
        /// The color of the haze as a Vector
        /// </summary>
        public Vector3 HazeColorVector
        {
            get
            {
                return m_data.HazeColor.XYZ;
            }
            set
            {
                if (m_data.HazeColor.XYZ != value)
                {
                    m_data.HazeColor.XYZ = value;

                    m_material.SetUserUniform(m_data);
                }
            }
        }

        public AtmospherePostEffect()
        {
            m_data = new ShaderData()
            {
                SunDir = new Vector4(Vector3.Normalized(new Vector3(0.0f, -1.0f, -1.0f)), 0.0f),
                Rayleigh = new Vector4(0.195f, 1.1f, 2.95f, 0.0f),
                Mie = new Vector4(0.4f, 0.4f, 0.4f, 0.75f),
                SunColor = new Vector4(1.6f, 1.4f, 1.0f, 0.0f),
                HazeColor = new Vector4(0.8f, 0.5f, 0.25f, 0.0f)
            };

            m_quadVertex = VertexShader.LoadVertexShader("[INTERNAL]Quad");
            m_atmospherePixel = PixelShader.LoadPixelShader("[INTERNAL]PostAtmosphere");

            MaterialBuilder material = new MaterialBuilder()
            {
                VertexShader = m_quadVertex,
                PixelShader = m_atmospherePixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None,
                UBOBuffer = m_data
            };

            m_material = Material.CreateMaterial(material);
        }

        /// <summary>
        /// Called when the post effect need to be run
        /// </summary>
        /// <param name="a_renderTexture">The target <see cref="IcarianEngine.Rendering.IRenderTexture" /></param>
        /// <param name="a_samplers">Samplers used by the RenderPipeline</param>
        public override void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers)
        {
            RenderCommand.BindRenderTexture(a_renderTexture);
            RenderCommand.BindMaterial(m_material);

            RenderCommand.PushTexture(0, a_samplers[0]);
            RenderCommand.PushTexture(1, a_samplers[3]);

            RenderCommand.DrawMaterial();
        }
        
        /// <summary>
        /// Disposes of the AtmospherePostEffect
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the AtmospherePostEffect is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether it is being called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_material != null)
            {
                if(a_disposing)
                {
                    m_material.Dispose();

                    m_quadVertex.Dispose();
                    m_atmospherePixel.Dispose();
                }
                else
                {
                    Logger.IcarianWarning("AtmospherePostEffect Failed to Dispose");
                }

                m_material = null;
            }
            else
            {
                Logger.IcarianError("Multiple AtmospherePostEffect Dispose");
            }
        }
        ~AtmospherePostEffect()
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