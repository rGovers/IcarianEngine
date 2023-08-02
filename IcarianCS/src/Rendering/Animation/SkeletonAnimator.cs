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

        public override void Update(float a_deltaTime)
        {
            AnimationController controller = AnimationController;

            if (controller != null)
            {
                controller.Update(this, a_deltaTime);

                foreach (Bone bone in m_skeleton.Bones)
                {
                    controller.UpdateObject(this, bone.Name, a_deltaTime);
                }
            }
        }  
    }
}