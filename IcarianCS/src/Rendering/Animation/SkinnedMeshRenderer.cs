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
                return m_skeletonBufferAddr != uint.MaxValue;
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

            m_skeletonBufferAddr = CreateSkeletonBuffer();

            SkinnedMeshRendererDef def = SkinnedMeshRendererDef;
            if (def != null)
            {
                if (def.MaterialDef != null)
                {
                    Material = AssetLibrary.GetMaterial(def.MaterialDef);
                }
                if (!string.IsNullOrWhiteSpace(def.ModelPath))
                {
                    Model = AssetLibrary.LoadSkinnedModel(def.ModelPath);
                }
                if (!string.IsNullOrWhiteSpace(def.SkeletonPath))
                {
                    SetSkeleton(AssetLibrary.LoadSkeleton(def.SkeletonPath));
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

            Transform.SetMatrix(a_bone.BindingPose);

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