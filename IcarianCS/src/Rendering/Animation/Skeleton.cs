using System.Collections.Generic;
using System.Runtime.CompilerServices;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public struct Bone
    {
        public string Name;
        public uint Parent;
        public Matrix4 BindingPose;
    }

    struct BoneData
    {
        public string[] Names;
        public uint[] Parents;
        public float[][] BindingPoses;
    }

    public class Skeleton
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static BoneData LoadBoneData(string a_path);

        string m_name;

        Bone[] m_bones;

        public string Name
        {
            get
            {
                return m_name;
            }
        }
        public IEnumerable<Bone> Bones
        {
            get
            {
                return m_bones;
            }
        }

        public Skeleton(string a_name)
        {
            m_name = a_name;

            m_bones = new Bone[0];
        }

        public static Skeleton LoadSkeleton(string a_path)
        {
            BoneData data = LoadBoneData(a_path);
            if (data.Names == null)
            {
                return null;
            }
            
            uint count = (uint)data.Names.Length;
            if (count > 0)
            {
                Skeleton skeleton = new Skeleton(a_path);
                skeleton.m_bones = new Bone[count];

                Bone bone;
                for (uint i = 0; i < count; ++i)
                {
                    bone.Name = data.Names[i];
                    bone.Parent = data.Parents[i];
                    bone.BindingPose = new Matrix4
                    (
                        data.BindingPoses[i][0],  data.BindingPoses[i][1],  data.BindingPoses[i][2],  data.BindingPoses[i][3],
                        data.BindingPoses[i][4],  data.BindingPoses[i][5],  data.BindingPoses[i][6],  data.BindingPoses[i][7],
                        data.BindingPoses[i][8],  data.BindingPoses[i][9],  data.BindingPoses[i][10], data.BindingPoses[i][11],
                        data.BindingPoses[i][12], data.BindingPoses[i][13], data.BindingPoses[i][14], data.BindingPoses[i][15]
                    );

                    skeleton.m_bones[i] = bone;
                }

                return skeleton;
            }

            return null;
        }

        public Matrix4 GetBoneBindingPose(string a_name)
        {
            uint index = GetIndex(a_name);
            if (index != uint.MaxValue)
            {
                return GetBoneBindingPose(index);
            }

            return Matrix4.Identity;
        }
        public Matrix4 GetBoneBindingPose(uint a_index)
        {
            return m_bones[(int)a_index].BindingPose;
        }

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
        public uint GetParentIndex(uint a_index)
        {
            return m_bones[(int)a_index].Parent;
        }
        public uint GetParentIndex(string a_name)
        {
            uint index = GetIndex(a_name);
            if (index != uint.MaxValue)
            {
                return GetParentIndex(index);
            }

            return uint.MaxValue;
        }
    };
}