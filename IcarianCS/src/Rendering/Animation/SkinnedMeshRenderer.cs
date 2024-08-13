// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

        /// @cond INTERNAL

        struct BoneData
        {
            public uint TransformAddr;
            public Matrix4 InverseBindPose;
        }

        // @endcond

        bool       m_visible = true;

        uint       m_rendererBufferAddr = uint.MaxValue;
        uint       m_skeletonBufferAddr = uint.MaxValue;

        GameObject m_root = null;
        Skeleton   m_skeleton = null;
        Material   m_material = null;
        Model      m_model = null;

        /// <summary>
        /// Whether the SkinnedMeshRenderer has been disposed
        /// </summary>
        public bool IsDisposed 
        {
            get
            {
                return m_skeletonBufferAddr == uint.MaxValue;
            }
        }
        /// <summary>
        /// The SkinnedMeshRendererDef used to create this SkinnedMeshRenderer
        /// </summary>
        public SkinnedMeshRendererDef SkinnedMeshRendererDef
        {
            get
            {
                return Def as SkinnedMeshRendererDef;
            }
        }

        /// <summary>
        /// Whether the SkinnedMeshRenderer is visible
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

        /// <summary>
        /// The Material used by the SkinnedMeshRenderer
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
        /// The Skeleton used by the SkinnedMeshRenderer
        /// </summary>
        public Skeleton Skeleton
        {
            get
            {
                return m_skeleton;
            }
        }
        /// <summary>
        /// The Model used by the SkinnedMeshRenderer
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

        /// <summary>
        /// Called when the SkinnedMeshRenderer is created
        /// </summary>
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
                    Model = AssetLibrary.LoadSkinnedModel(def.ModelPath, def.Index);
                }
            }
        }

        void GenerateBone(Bone a_bone, Matrix4 a_inverse, Transform a_parent, ref BoneData[] a_data)
        {
            GameObject boneObject = GameObject.Instantiate();
            boneObject.Name = a_bone.Name;
            boneObject.Transform.Parent = a_parent;

            Matrix4 bindingPose = a_bone.BindingPose;
            Matrix4 invPose = Matrix4.Inverse(bindingPose);

            boneObject.Transform.SetMatrix(bindingPose * a_inverse);

            a_data[a_bone.Index] = new BoneData()
            {
                TransformAddr = boneObject.Transform.InternalAddr,
                InverseBindPose = invPose
            };

            IEnumerable<Bone> children = m_skeleton.GetChildren(a_bone);
            foreach (Bone child in children)
            {
                GenerateBone(child, invPose, boneObject.Transform, ref a_data);
            }
        }

        /// <summary>
        /// Sets the Skeleton used by the SkinnedMeshRenderer
        /// </summary>
        /// <param name="a_skeleton">The Skeleton to use</param>
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

                BoneData[] data = new BoneData[m_skeleton.BoneCount];

                foreach (Bone bone in m_skeleton.RootBones)
                {
                    GenerateBone(bone, Matrix4.Identity, m_root.Transform, ref data);
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

        /// <summary>
        /// Disposes the SkinnedMeshRenderer
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the SkinnedMeshRenderer is disposed
        /// </summary>
        /// <param name="a_disposing">Whether the SkinnedMeshRenderer is being disposed</param>
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

                    Model = null;
                    Material = null;
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