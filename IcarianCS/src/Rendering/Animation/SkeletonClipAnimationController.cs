using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public class SkeletonClipAnimationController : AnimationController
    {
        AnimationClip m_clip;

        float         m_time;

        public float Time
        {
            get
            {
                return m_time;
            }
        }

        public SkeletonClipAnimationControllerDef SkeletonClipAnimationControllerDef
        {
            get
            {
                return ControllerDef as SkeletonClipAnimationControllerDef;
            }
        }

        public override void Init()
        {
            SkeletonClipAnimationControllerDef def = SkeletonClipAnimationControllerDef;
            if (def != null)
            {
                Logger.IcarianWarning("Need to implement loading of animation clips");
            }
        }

        public override bool Update(Animator a_animator, double a_deltaTime)
        {
            m_time += (float)a_deltaTime;
            if (m_clip != null)
            {
                float clipDuration = m_clip.Duration;

                if (m_time >= clipDuration)
                {
                    m_time -= clipDuration;
                }
            }

            return true;
        }
        public override void UpdateObject(Animator a_animator, string a_object, double a_deltaTime)
        {
            if (m_clip != null)
            {
                Matrix4 mat = m_clip.GetTransform(a_object, m_time);

                a_animator.PushTransform(a_object, mat);
            }   
        }
    }
}