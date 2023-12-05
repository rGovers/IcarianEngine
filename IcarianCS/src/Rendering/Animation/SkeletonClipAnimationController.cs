using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Animation
{
    public class SkeletonClipAnimationController : AnimationController
    {
        AnimationClip m_clip;

        float         m_time;

        /// <summary>
        /// The time of the animation.
        /// </summary>
        public float Time
        {
            get
            {
                return m_time;
            }
        }
        /// <summary>
        /// The animation clip.
        /// </summary>
        public AnimationClip Clip
        {
            get
            {
                return m_clip;
            }
        }

        /// <summary>
        /// The def used to create the controller.
        /// </summary>
        public SkeletonClipAnimationControllerDef SkeletonClipAnimationControllerDef
        {
            get
            {
                return ControllerDef as SkeletonClipAnimationControllerDef;
            }
        }

        /// <summary>
        /// Called when the controller is created.
        /// </summary>
        public override void Init()
        {
            SkeletonClipAnimationControllerDef def = SkeletonClipAnimationControllerDef;
            if (def != null)
            {
                m_clip = AssetLibrary.LoadAnimationClip(def.ClipPath);
            }
        }

        /// <summary>
        /// Called to update the animation controller.
        /// </summary>
        /// <param name="a_animator">The animator.</param>
        /// <param name="a_deltaTime">The delta time.</param>
        /// <returns>Continue to Object Update.</returns>
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
        /// <summary>
        /// Called to update the object.
        /// </summary>
        /// <param name="a_animator">The animator.</param>
        /// <param name="a_object">The object.</param>
        /// <param name="a_deltaTime">The delta time.</param>
        public override void UpdateObject(Animator a_animator, string a_object, double a_deltaTime)
        {
            SkeletonAnimator animator = a_animator as SkeletonAnimator;
            if (animator == null)
            {
                return;
            }

            if (m_clip != null)
            {
                Matrix4 mat = m_clip.GetTransform(animator.Skeleton, a_object, m_time);

                animator.PushTransform(a_object, mat);
            }   
        }
    }
}