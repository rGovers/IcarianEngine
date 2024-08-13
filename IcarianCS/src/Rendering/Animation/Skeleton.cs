// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using IcarianEngine.Maths;

#include "EngineSkeletonInteropStructures.h"

namespace IcarianEngine.Rendering.Animation
{
    public struct Bone
    {
        /// <summary>
        /// The name of the bone.
        /// </summary>
        public string Name;
        /// <summary>
        /// The index of the bone.
        /// </summary>
        public uint Index;
        /// <summary>
        /// The index of the parent bone.
        /// </summary>
        public uint Parent;
        /// <summary>
        /// The binding pose matrix of the bone.
        /// </summary>
        public Matrix4 BindingPose;
    }

    public class Skeleton
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static RuntimeImportBoneData LoadBoneData(string a_path);

        Bone[] m_bones;

        /// <summary>
        /// The bones in the skeleton.
        /// </summary>
        public IEnumerable<Bone> Bones
        {
            get
            {
                return m_bones;
            }
        }

        /// <summary>
        /// The number of bones in the skeleton.
        /// </summary>
        public uint BoneCount
        {
            get
            {
                return (uint)m_bones.Length;
            }
        }

        /// <summary>
        /// The root bones in the skeleton.
        /// </summary>
        public IEnumerable<Bone> RootBones
        {
            get
            {
                foreach (Bone bone in m_bones)
                {
                    if (bone.Parent == uint.MaxValue)
                    {
                        yield return bone;
                    }
                }
            }
        }

        /// <summary>
        /// Gets the children of the specified bone.
        /// </summary>
        /// <param name="a_parent">The parent bone.</param>
        /// <returns>The children of the specified bone.</returns>
        public IEnumerable<Bone> GetChildren(Bone a_parent)
        {
            foreach (Bone bone in m_bones)
            {
                if (bone.Parent == a_parent.Index)
                {
                    yield return bone;
                }
            }
        }

        /// <summary>
        /// Loads a skeleton from a file.
        /// </summary>
        /// Supports the following file formats:
        ///     .dae
        ///     .fbx
        ///     .glb
        ///     .gltf
        /// <param name="a_path">The path to the file.</param>
        /// <returns>The skeleton loaded from the file. Null on error.</returns>
        /// @see IcarianEngine.AssetLibrary.LoadSkeleton
        public static Skeleton LoadSkeleton(string a_path)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianError("Invalid path to skeleton file.");

                return null;
            }

            RuntimeImportBoneData data = LoadBoneData(a_path);
            if (data.Names == null)
            {
                Logger.IcarianWarning($"Failed to load skeleton from file: {a_path}");

                return null;
            }
            
            uint count = (uint)data.Names.Length;
            if (count > 0)
            {
                Skeleton skeleton = new Skeleton();
                skeleton.m_bones = new Bone[count];

                string[] names = data.Names as string[];
                uint[] parents = data.Parents as uint[];
                float[][] bindingPoses = data.BindPoses as float[][];

                Bone bone;
                for (uint i = 0; i < count; ++i)
                {
                    bone.Name = names[i];
                    bone.Index = i;
                    bone.Parent = parents[i];
                    bone.BindingPose = new Matrix4
                    (
                        bindingPoses[i][0],  bindingPoses[i][1],  bindingPoses[i][2],  bindingPoses[i][3],
                        bindingPoses[i][4],  bindingPoses[i][5],  bindingPoses[i][6],  bindingPoses[i][7],
                        bindingPoses[i][8],  bindingPoses[i][9],  bindingPoses[i][10], bindingPoses[i][11],
                        bindingPoses[i][12], bindingPoses[i][13], bindingPoses[i][14], bindingPoses[i][15]
                    );

                    skeleton.m_bones[i] = bone;
                }

                return skeleton;
            }

            return null;
        }

        /// <summary>
        /// Gets the bone binding pose matrix for the specified bone.
        /// </summary>
        /// <param name="a_name">The name of the bone.</param>
        /// <returns>The bone binding pose matrix for the specified bone.</returns>
        public Matrix4 GetBoneBindingPose(string a_name)
        {
            uint index = GetIndex(a_name);
            if (index != uint.MaxValue)
            {
                return GetBoneBindingPose(index);
            }

            return Matrix4.Identity;
        }
        /// <summary>
        /// Gets the bone binding pose matrix for the specified bone.
        /// </summary>
        /// <param name="a_index">The index of the bone.</param>
        /// <returns>The bone binding pose matrix for the specified bone.</returns>
        public Matrix4 GetBoneBindingPose(uint a_index)
        {
            return m_bones[(int)a_index].BindingPose;
        }

        /// <summary>
        /// Gets the index of the specified bone.
        /// </summary>
        /// <param name="a_name">The name of the bone.</param>
        /// <returns>The index of the specified bone. uint.MaxValue on error.</returns>
        public uint GetIndex(string a_name)
        {
            for (uint i = 0; i < m_bones.Length; ++i)
            {
                if (m_bones[(int)i].Name == a_name)
                {
                    return i;
                }
            }

            return uint.MaxValue;
        }
        /// <summary>
        /// Gets the index of the parent bone of the specified bone.
        /// </summary>
        /// <param name="a_index">The index of the bone.</param>
        /// <returns>The index of the parent bone of the specified bone.</returns>
        public uint GetParentIndex(uint a_index)
        {
            return m_bones[(int)a_index].Parent;
        }
        /// <summary>
        /// Gets the index of the parent bone of the specified bone.
        /// </summary>
        /// <param name="a_name">The name of the bone.</param>
        /// <returns>The index of the parent bone of the specified bone. uint.MaxValue on error.</returns>
        public uint GetParentIndex(string a_name)
        {
            uint index = GetIndex(a_name);
            if (index != uint.MaxValue)
            {
                return GetParentIndex(index);
            }

            return uint.MaxValue;
        }

        /// <summary>
        /// Gets the local translation of the specified bone.
        /// </summary>
        /// <param name="a_name">The name of the bone.</param>
        /// <returns>The local translation of the specified bone.</returns>
        public Vector3 GetLocalTranslation(string a_name)
        {
            uint index = GetIndex(a_name);
            if (index != uint.MaxValue)
            {
                return GetLocalTranslation(index);
            }

            return Vector3.Zero;
        }
        /// <summary>
        /// Gets the local translation of the specified bone.
        /// </summary>
        /// <param name="a_index">The index of the bone.</param>
        /// <returns>The local translation of the specified bone.</returns>       
        public Vector3 GetLocalTranslation(uint a_index)
        {
            Matrix4 bindingPose = GetBoneBindingPose(a_index);
            uint parentIndex = GetParentIndex(a_index);
            
            if (parentIndex != uint.MaxValue)
            {
                Matrix4 parentBindingPose = GetBoneBindingPose(parentIndex);
                Matrix4 invBindPose = Matrix4.Inverse(parentBindingPose);

                bindingPose = bindingPose * invBindPose;
            }

            Vector3 translation;
            Quaternion rotation;
            Vector3 scale;

            Matrix4.Decompose(bindingPose, out translation, out rotation, out scale);

            return translation;
        }
        /// <summary>
        /// Gets the local rotation of the specified bone.
        /// </summary>
        /// <param name="a_name">The name of the bone.</param>
        /// <returns>The local rotation of the specified bone.</returns>
        public Quaternion GetLocalRotation(string a_name)
        {
            uint index = GetIndex(a_name);
            if (index != uint.MaxValue)
            {
                return GetLocalRotation(index);
            }

            return Quaternion.Identity;
        }
        /// <summary>
        /// Gets the local rotation of the specified bone.
        /// </summary>
        /// <param name="a_index">The index of the bone.</param>
        /// <returns>The local rotation of the specified bone.</returns>
        public Quaternion GetLocalRotation(uint a_index)
        {
            Matrix4 bindingPose = GetBoneBindingPose(a_index);
            uint parentIndex = GetParentIndex(a_index);

            if (parentIndex != uint.MaxValue)
            {
                Matrix4 parentBindingPose = GetBoneBindingPose(parentIndex);
                Matrix4 invBindPose = Matrix4.Inverse(parentBindingPose);

                bindingPose = bindingPose * invBindPose;
            }

            Vector3 translation;
            Quaternion rotation;
            Vector3 scale;

            Matrix4.Decompose(bindingPose, out translation, out rotation, out scale);

            return rotation;
        }
    };
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