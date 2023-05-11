using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public class MeshRenderer : Component, IDestroy
    {
        bool     m_disposed = false;

        bool     m_visible = true;

        uint     m_bufferAddr = uint.MaxValue;

        Model    m_model = null;

        Material m_material = null;

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr, uint a_materialAddr, uint a_modelAddr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void GenerateRenderStack(uint a_addr); 
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void DestroyRenderStack(uint a_addr); 

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public MeshRendererDef MeshRendererDef
        {
            get
            {
                return Def as MeshRendererDef;
            }
        }

        public bool Visible
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

        public Model Model
        {
            get
            {
                return m_model;
            }
            set
            {
                m_model = value;

                PushData();
            }
        }

        public Material Material
        {
            get
            {
                return m_material;
            }
            set
            {
                m_material = value;

                PushData();
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

        public override void Init()
        {
            base.Init();

            MeshRendererDef def = MeshRendererDef;
            if (def != null)
            {   
                Material = AssetLibrary.GetMaterial(def.MaterialDef);
                if (!string.IsNullOrWhiteSpace(def.ModelPath))
                {
                    Model = AssetLibrary.LoadModel(def.ModelPath);
                }   
            }
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

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