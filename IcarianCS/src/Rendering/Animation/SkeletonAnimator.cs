using IcarianEngine.Definitions;

namespace IcarianEngine.Rendering.Animation
{
    public class SkeletonAnimator : Animator
    {
        Skeleton m_skeleton;

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
            set
            {
                m_skeleton = value;
            }
        }

        public override void Init()
        {
            base.Init();
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