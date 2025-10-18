// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class MeshRenderer : Renderer, IDestroy
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint GenerateBuffer(uint a_transformAddr, uint a_materialAddr, uint a_meshAddr, uint a_indexCount); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DestroyBuffer(uint a_addr);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GenerateRenderStack(uint a_addr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DestroyRenderStack(uint a_addr); 

        bool     m_disposed = false;
        bool     m_visible = true;

        uint     m_bufferAddr = uint.MaxValue;
        uint     m_indexCount = uint.MaxValue;

        Mesh     m_mesh = null;

        Material m_material = null;

        /// <summary>
        /// Whether the MeshRenderer has been Disposed/Finalized
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Defintion.MeshRenderDef" /> used to create this MeshRenderer
        /// </summary>
        public MeshRendererDef MeshRendererDef
        {
            get
            {
                return Def as MeshRendererDef;
            }
        }

        /// <summary>
        /// Whether the MeshRenderer is visible
        /// </summary>
        public override bool Visible
        {
            get
            {
                return m_visible;
            }
            set
            {
                if (m_visible != value)
                {
                    if (m_visible && m_bufferAddr != uint.MaxValue)
                    {
                        DestroyRenderStack(m_bufferAddr);
                    }

                    m_visible = value;

                    if (m_visible && m_bufferAddr != uint.MaxValue)
                    {
                        GenerateRenderStack(m_bufferAddr);
                    }
                }
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Material" /> of the MeshRenderer
        /// </summary>
        public override Material Material
        {
            get
            {
                return m_material;
            }
            set
            {
                if (value != null)
                {
                    if (value.MaterialMode != MaterialMode.BaseMesh)
                    {
                        Logger.IcarianError($"Invalid MaterialMode of Material assigned to MeshRenderer: {value.MaterialMode}");

                        m_material = null;

                        PushData();

                        return;
                    }
                }

                if (m_material != value)
                {
                    m_material = value;

                    PushData();
                }
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Mesh" /> of the MeshRenderer
        /// </summary>
        public Mesh Mesh
        {
            get
            {
                return m_mesh;
            }
            set
            {
                if (m_mesh != value)
                {
                    m_mesh = value;

                    PushData();
                }
            }
        }

        /// <summary>
        /// The number of indices for the MeshRenderer to draw
        /// </summary
        /// uint.MaxValue is used for the MeshletCount to be used
        public uint IndexCount
        {
            get
            {
                return m_indexCount;
            }
            set
            {
                if (m_indexCount != value)
                {
                    m_indexCount = value;

                    PushData();
                }
            }
        }

        void PushData()
        {
            if (m_bufferAddr != uint.MaxValue)
            {
                if (m_visible)
                {
                    DestroyRenderStack(m_bufferAddr);
                }

                DestroyBuffer(m_bufferAddr);

                m_bufferAddr = uint.MaxValue;
            }

            if (m_material == null)
            {
                return;
            }

            if (m_mesh == null && m_indexCount == uint.MaxValue)
            {
                return;
            }

            if (m_indexCount <= 0)
            {
                return;
            }

            uint meshAddr = uint.MaxValue;
            if (m_mesh != null)
            {
                meshAddr = m_mesh.InternalAddr;
            }

            m_bufferAddr = GenerateBuffer(Transform.InternalAddr, m_material.InternalAddr, meshAddr, m_indexCount);

            if (m_visible)
            {
                GenerateRenderStack(m_bufferAddr);
            }
        }

        /// <summary>
        /// Called when the MeshRenderer is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            RendererDef def = RendererDef;
            if (def != null)
            {
                MeshRendererDef meshDef = def as MeshRendererDef;
                if (meshDef != null)
                {
                    IndexCount = meshDef.IndexCount;
                }

                Material = AssetLibrary.GetMaterial(def.MaterialDef);
            }
        }

        /// <summary>
        /// Disposes of the MeshRenderer
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the MeshRenderer is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    m_material = null;

                    if (m_bufferAddr != uint.MaxValue)
                    {
                        if (m_visible)
                        {
                            DestroyRenderStack(m_bufferAddr);
                        }

                        DestroyBuffer(m_bufferAddr);

                        m_bufferAddr = uint.MaxValue;
                    }
                }
                else
                {
                    Logger.IcarianWarning("MeshRenderer not Disposed");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("MeshRenderer already Disposed");
            }
        }
        ~MeshRenderer()
        {
            Dispose(false);
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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
