using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public class SkinnedMeshRenderer : Renderer, IDestroy
    {   
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateSkeletonBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroySkeletonBuffer(uint a_bufferAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void ClearSkeletonBuffer(uint a_bufferAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushBoneData(uint a_bufferAddr, uint a_transformIndex, float[] a_inverseBindPose);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr, uint a_materialAddr, uint a_modelAddr, uint a_skeletonAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_bufferAddr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void GenerateRenderStack(uint a_bufferAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyRenderStack(uint a_bufferAddr);

        bool       m_visible = true;

        uint       m_rendererBufferAddr = uint.MaxValue;
        uint       m_skeletonBufferAddr = uint.MaxValue;

        GameObject m_root = null;
        Skeleton   m_skeleton = null;
        Material   m_material = null;
        Model      m_model = null;

        public bool IsDisposed 
        {
            get
            {
                return m_skeletonBufferAddr == uint.MaxValue;
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
                if (m_visible != value)
                {
                    if (m_visible && m_rendererBufferAddr != uint.MaxValue)
                    {
                        DestroyRenderStack(m_rendererBufferAddr);
                    }

                    m_visible = value;

                    if (m_visible && m_rendererBufferAddr != uint.MaxValue)
                    {
                        GenerateRenderStack(m_rendererBufferAddr);
                    }
                }
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
                if (m_material != value)
                {
                    m_material = value;

                    PushData();
                }
            }
        }

        public Skeleton Skeleton
        {
            get
            {
                return m_skeleton;
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
                if (m_model != value)
                {
                    m_model = value;

                    PushData();
                }
            }
        }

        void PushData()
        {
            if (m_rendererBufferAddr != uint.MaxValue)
            {
                if (m_visible)
                {
                    DestroyRenderStack(m_rendererBufferAddr);
                }

                DestroyBuffer(m_rendererBufferAddr);

                m_rendererBufferAddr = uint.MaxValue;
            }

            if (m_model != null && m_material != null)
            {
                m_rendererBufferAddr = GenerateBuffer(Transform.InternalAddr, m_material.InternalAddr, m_model.InternalAddr, m_skeletonBufferAddr);

                if (m_visible)
                {
                    GenerateRenderStack(m_rendererBufferAddr);
                }
            }
        }

        public override void Init()
        {
            base.Init();

            m_skeletonBufferAddr = CreateSkeletonBuffer();

            SkinnedMeshRendererDef def = SkinnedMeshRendererDef;
            if (def != null)
            {
                if (!string.IsNullOrWhiteSpace(def.SkeletonPath))
                {
                    SetSkeleton(AssetLibrary.LoadSkeleton(def.SkeletonPath));
                }
                if (def.MaterialDef != null)
                {
                    Material = AssetLibrary.GetMaterial(def.MaterialDef);
                }
                if (!string.IsNullOrWhiteSpace(def.ModelPath))
                {
                    Model = AssetLibrary.LoadSkinnedModel(def.ModelPath);
                }
            }
        }

        struct BoneData
        {
            public uint TransformAddr;
            public Matrix4 InverseBindPose;
        }

        void GenerateBone(Bone a_bone, Transform a_parent, ref Dictionary<uint, BoneData> a_data)
        {
            GameObject boneObject = GameObject.Instantiate();
            boneObject.Name = a_bone.Name;
            boneObject.Transform.Parent = a_parent;

            Matrix4 invPose = Matrix4.Inverse(a_bone.BindingPose);

            BoneData data = new BoneData()
            {
                TransformAddr = boneObject.Transform.InternalAddr,
                InverseBindPose = invPose
            };

            boneObject.Transform.SetMatrix(a_bone.BindingPose);

            a_data.Add(a_bone.Index, data);

            IEnumerable<Bone> children = m_skeleton.GetChildren(a_bone);
            foreach (Bone child in children)
            {
                GenerateBone(child, boneObject.Transform, ref a_data);
            }
        }

        public void SetSkeleton(Skeleton a_skeleton)
        {
            if (m_root != null && !m_root.IsDisposed)
            {
                m_root.Dispose();
            }

            ClearSkeletonBuffer(m_skeletonBufferAddr);

            m_skeleton = a_skeleton;

            if (m_skeleton != null)
            {
                m_root = GameObject.Instantiate();
                m_root.Name = "Root";
                m_root.Transform.Parent = Transform;

                Dictionary<uint, BoneData> data = new Dictionary<uint, BoneData>();

                foreach (Bone bone in m_skeleton.RootBones)
                {
                    GenerateBone(bone, m_root.Transform, ref data);
                }

                foreach (Bone b in m_skeleton.Bones)
                {
                    BoneData boneData = data[b.Index];
                    PushBoneData(m_skeletonBufferAddr, boneData.TransformAddr, boneData.InverseBindPose.ToArray());
                }
            }

            // Cant guarantee order of execution so animators and renderers trigger refresh
            IEnumerable<SkeletonAnimator> animators = GameObject.GetComponents<SkeletonAnimator>();
            foreach (SkeletonAnimator anim in animators)
            {
                anim.RefreshSkeleton();
            }
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        protected virtual void Dispose(bool a_disposing)
        {
            if (m_skeletonBufferAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    if (m_root != null && !m_root.IsDisposed)
                    {
                        m_root.Dispose();
                    }

                    m_model = null;
                    m_material = null;
                    m_skeleton = null;

                    DestroySkeletonBuffer(m_skeletonBufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("SkinnedMeshRenderer Failed to Dispose");
                }

                m_skeletonBufferAddr = uint.MaxValue;
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