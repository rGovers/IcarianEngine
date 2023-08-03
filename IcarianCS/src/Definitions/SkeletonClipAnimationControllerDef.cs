using IcarianEngine.Rendering.Animation;

namespace IcarianEngine.Definitions
{
    public class SkeletonClipAnimationControllerDef : AnimationControllerDef
    {
        public string ClipPath;

        public SkeletonClipAnimationControllerDef()
        {
            ControllerType = typeof(SkeletonClipAnimationController);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ControllerType != typeof(SkeletonClipAnimationController) && !ControllerType.IsSubclassOf(typeof(SkeletonClipAnimationController)))
            {
                Logger.IcarianError($"SkeletonClipAnimationControllerDef Invalid ControllerType: {ControllerType}");

                return;
            }

            if (string.IsNullOrWhiteSpace(ClipPath))
            {
                Logger.IcarianWarning($"SkeletonClipAnimationControllerDef invalid ClipPath");
            }
        }
    }
}