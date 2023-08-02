using IcarianEngine.Rendering.Animation;

namespace IcarianEngine.Definitions
{
    public class AnimatorDef : ComponentDef
    {
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