using IcarianEngine.Rendering.Animation;

namespace IcarianEngine.Definitions
{
    public class AnimatorDef : ComponentDef
    {
        [EditorTooltip("AnimationControllerDef that will be used to control the animation.")]
        public AnimationControllerDef ControllerDef;

        public AnimatorDef()
        {
            ComponentType = typeof(Animator);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (!ComponentType.IsSubclassOf(typeof(Animator)))
            {
                Logger.IcarianError($"AnimatorDef Invalid ComponentType: {ComponentType}");

                return;
            }

            if (ControllerDef == null)
            {
                Logger.IcarianWarning($"AnimatorDef ControllerDef is null");

                return;
            }
        }
    }
}