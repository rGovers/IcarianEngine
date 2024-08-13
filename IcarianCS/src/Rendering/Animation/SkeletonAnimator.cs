// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Collections.Generic;
using System.Runtime.CompilerServices;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public class SkeletonAnimator : Animator
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PushTransform(uint a_buffer, string a_object, Matrix4 a_transform);

        uint                           m_buffer = uint.MaxValue;
        Skeleton                       m_skeleton = null;
        Dictionary<string, GameObject> m_bones = new Dictionary<string, GameObject>();

        public SkeletonAnimatorDef SkeletonAnimatorDef
        {
            get
            {
                return Def as SkeletonAnimatorDef;
            }
        }

        public Skeleton Skeleton
        {
            get
            {
                return m_skeleton;
            }
        }

        public override void Init()
        {
            base.Init();

            SkeletonAnimatorDef def = SkeletonAnimatorDef;
            if (def != null)
            {
                if (!string.IsNullOrWhiteSpace(def.SkeletonPath))
                {
                    SetSkeleton(AssetLibrary.LoadSkeleton(def.SkeletonPath));
                }
            }
        }

        public void SetSkeleton(Skeleton a_skeleton)
        {
            m_skeleton = a_skeleton;

            if (!Application.IsEditor)
            {
                RefreshSkeleton();
            }
        }

        internal void RefreshSkeleton()
        {
            m_bones.Clear();

            if (m_skeleton != null)
            {
                GameObject root = GameObject.GetChildWithName("Root");

                if (root != null)
                {
                    foreach (Bone bone in m_skeleton.Bones)
                    {   
                        // Should probably not use a recursive search as it should match the hierarchy
                        // but lazy for now
                        GameObject boneObject = root.GetChildWithName(bone.Name, true);

                        if (boneObject != null)
                        {
                            m_bones.Add(bone.Name, boneObject);
                        }
                    }
                }
            }
        }

        public void PushTransform(string a_object, Matrix4 a_transform)
        {
            if (!Application.IsEditor)
            {
                GameObject boneObject = m_bones[a_object];
                boneObject.Transform.SetMatrix(a_transform);
            }
            else
            {
                PushTransform(m_buffer, a_object, a_transform);
            }
        }

        public override void Update(double a_deltaTime)
        {
            AnimationController controller = AnimationController;

            if (controller != null)
            {
                if (controller.Update(this, a_deltaTime))
                {
                    if (m_skeleton != null)
                    {
                        foreach (Bone bone in m_skeleton.Bones)
                        {
                            controller.UpdateObject(this, bone.Name, a_deltaTime);
                        }
                    }
                }
            }
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