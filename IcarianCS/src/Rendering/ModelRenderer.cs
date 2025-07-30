// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    // Shot myself in the foot calling it MeshRenderer and considering have minimal users renamed it to ModelRenderer for Graphics pipeline
    // The new MeshRenderer is the Compute based Task/Mesh shaders
    public class ModelRenderer : Renderer, IDestroy
    {
        // NOTE: This was fine at the time of writing but now is starting to seem like code smell consider moving these 4 functions down the line
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint GenerateBuffer(uint a_transformAddr, uint a_materialAddr, uint a_modelAddr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DestroyBuffer(uint a_addr);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GenerateRenderStack(uint a_addr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DestroyRenderStack(uint a_addr); 

        bool     m_disposed = false;
        bool     m_visible = true;

        uint     m_bufferAddr = uint.MaxValue;

        Model    m_model = null;

        Material m_material = null;

        /// <summary>
        /// Whether the ModelRenderer has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Definition.ModelRendererDef" /> used to create this ModelRenderer
        /// </summary>
        public ModelRendererDef ModelRendererDef
        {
            get
            {
                return Def as ModelRendererDef;
            }
        }

        /// <summary>
        /// Whether the ModelRender is visible
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
        /// The <see cref="IcarianEngine.Rendering.Material" /> of the ModelRenderer
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
                    if (value.MaterialMode != MaterialMode.BaseVertex)
                    {
                        Logger.IcarianError($"Invalid MaterialMode of Material assigned to ModelRenderer: {value.MaterialMode}");

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
        /// The <see cref="IcarianEngine.Rendering.Model" /> of the ModelRenderer
        /// </summary>
        public Model Model
        {
            get
            {
                return m_model;
            }
            set
            {
                if (m_model != value)
                {
                    m_model = value;

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

            if (m_model != null && m_material != null)
            {
                m_bufferAddr = GenerateBuffer(Transform.InternalAddr, m_material.InternalAddr, m_model.InternalAddr);

                if (m_visible)
                {
                    GenerateRenderStack(m_bufferAddr);
                }
            }
        }

        /// <summary>
        /// Called when the ModelRenderer is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            RendererDef def = RendererDef;
            if (def != null)
            {   
                Material = AssetLibrary.GetMaterial(def.MaterialDef);

                ModelRendererDef modelDef = ModelRendererDef;
                if (modelDef != null && !string.IsNullOrWhiteSpace(modelDef.ModelPath))
                {
                    Model = AssetLibrary.LoadModel(modelDef.ModelPath, modelDef.Index);
                }   
            }
        }

        /// <summary>
        /// Disposes of the ModelRenderer
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the ModelRenderer is Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    m_model = null;
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
                    Logger.IcarianWarning("ModelRenderer not Disposed");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("ModelRenderer already Disposed");
            }
        }
        ~ModelRenderer()
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
