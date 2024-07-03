using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class MeshRenderer : Renderer, IDestroy
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr, uint a_materialAddr, uint a_modelAddr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void GenerateRenderStack(uint a_addr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyRenderStack(uint a_addr); 

        bool     m_disposed = false;

        bool     m_visible = true;

        uint     m_bufferAddr = uint.MaxValue;

        Model    m_model = null;

        Material m_material = null;

        /// <summary>
        /// Whether the MeshRenderer has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The Def used to create this MeshRenderer
        /// </summary>
        public MeshRendererDef MeshRendererDef
        {
            get
            {
                return Def as MeshRendererDef;
            }
        }

        /// <summary>
        /// Whether the MeshRender is visible
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
                if (m_material != value)
                {
                    m_material = value;

                    PushData();
                }
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Model" /> of the MeshRender
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
        /// Called when the MeshRenderer is created
        /// </summary>
        public override void Init()
        {
            base.Init();

            MeshRendererDef def = MeshRendererDef;
            if (def != null)
            {   
                Material = AssetLibrary.GetMaterial(def.MaterialDef);
                if (!string.IsNullOrWhiteSpace(def.ModelPath))
                {
                    Model = AssetLibrary.LoadModel(def.ModelPath, def.Index);
                }   
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
        /// Called when the MeshRenderer is Dispose/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether was called from Dispose</param>
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
                    Logger.IcarianWarning("MeshRenderer Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("Multiple MeshRenderer Dispose");
            }
        }
        ~MeshRenderer()
        {
            Dispose(false);
        }
    }
}