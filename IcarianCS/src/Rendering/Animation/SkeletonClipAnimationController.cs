// Icarian Engine - C# Game Engine
// 
// License at end of file.

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