using System;
using IcarianEngine.Definitions;

namespace IcarianEngine.Rendering.Animation
{
    public class SkinnedMeshRenderer : Renderer, IDestroy
    {   
        bool     m_disposed = false;

        bool     m_visible = true;

        uint     m_bufferAddr = uint.MaxValue;

        Skeleton m_skeleton = null;
        Material m_material = null;
        Model    m_model = null;

        public bool IsDisposed 
        {
            get
            {
                return m_disposed;
            }
        }
        public SkinnedMeshRendererDef SkinnedMeshRendererDef
        {
            get
            {
                return Def as SkinnedMeshRendererDef;
            }
        }

        public override bool Visible 
        {
            get
            {
                return m_visible;
            }
            set
            {
                m_visible = value;
            }
        }

        public override Material Material 
        {
            get
            {
                return m_material;
            }
            set
            {
                m_material = value;
            }
        }

        public Skeleton Skeleton
        {
            get
            {
                return m_skeleton;
            }
            set
            {
                m_skeleton = value;
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
            }
        }

        public override void Init()
        {
            base.Init();

            SkinnedMeshRendererDef def = SkinnedMeshRendererDef;
            if (def != null)
            {
                Material = AssetLibrary.GetMaterial(def.MaterialDef);
                if (!string.IsNullOrWhiteSpace(def.ModelPath))
                {
                    Logger.IcarianWarning("Implement skinned model loading");
                }
                if (!string.IsNullOrWhiteSpace(def.SkeletonPath))
                {
                    Logger.IcarianWarning("Implement skeleton loading");
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
            if (!m_disposed)
            {
                if (a_disposing)
                {
                    m_model = null;
                    m_material = null;
                    m_skeleton = null;

                    if (m_bufferAddr != uint.MaxValue)
                    {
                        m_bufferAddr = uint.MaxValue;
                    }
                }
                else
                {
                    Logger.IcarianWarning("SkinnedMeshRenderer Failed to Dispose");
                }

                m_disposed = true;
            }
            else
            {
                Logger.IcarianError("Multiple SkinnedMeshRenderer Dispose");
            }
        }
        ~SkinnedMeshRenderer()
        {
            Dispose(false);
        }
    }
}