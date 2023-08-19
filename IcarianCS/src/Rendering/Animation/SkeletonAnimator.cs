using System.Collections.Generic;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public class SkeletonAnimator : Animator
    {
        GameObject                     m_root;
        Skeleton                       m_skeleton;
        Dictionary<string, GameObject> m_bones;

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

            m_root = null;
            m_bones = new Dictionary<string, GameObject>();

            SkeletonAnimatorDef def = SkeletonAnimatorDef;
            if (def != null)
            {
                if (!string.IsNullOrWhiteSpace(def.SkeletonPath))
                {
                    SetSkeleton(AssetLibrary.LoadSkeleton(def.SkeletonPath));
                }
            }
        }

        void GenerateBone(Bone a_bone)
        {
            GameObject boneObject = GameObject.Instantiate();
            boneObject.Name = a_bone.Name;
            boneObject.Transform.Parent = m_root.Transform;

            m_bones.Add(a_bone.Name, boneObject);

            Transform.SetMatrix(a_bone.BindingPose);

            IEnumerable<Bone> children = m_skeleton.GetChildren(a_bone);
            foreach (Bone child in children)
            {
                GenerateBone(child);
            }
        }

        public void SetSkeleton(Skeleton a_skeleton)
        {
            if (m_root != null)
            {
                m_root.Dispose();
            }

            m_bones.Clear();

            m_skeleton = a_skeleton;

            if (m_skeleton != null)
            {
                m_root = GameObject.Instantiate();
                m_root.Name = "Root";
                m_root.Transform.Parent = Transform;

                foreach (Bone bone in m_skeleton.RootBones)
                {
                    GenerateBone(bone);
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
                Logger.IcarianError("PushTransform not implemented in editor");
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