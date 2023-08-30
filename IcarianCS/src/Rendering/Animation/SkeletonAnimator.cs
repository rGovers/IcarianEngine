using System.Collections.Generic;
using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public class SkeletonAnimator : Animator
    {
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