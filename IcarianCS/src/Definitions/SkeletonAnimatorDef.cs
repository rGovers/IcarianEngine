using IcarianEngine.Rendering.Animation;

namespace IcarianEngine.Definitions
{
    public class SkeletonAnimatorDef : AnimatorDef
    {
        public string SkeletonPath;

        public SkeletonAnimatorDef()
        {
            ComponentType = typeof(SkeletonAnimator);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(SkeletonAnimator) && !ComponentType.IsSubclassOf(typeof(SkeletonAnimator)))
            {
                Logger.IcarianError($"SkeletonAnimatorDef Invalid ComponentType: {ComponentType}");

                return;
            }

            if (string.IsNullOrWhiteSpace(SkeletonPath))
            {
                Logger.IcarianWarning($"SkeletonAnimatorDef SkeletonPath is null or whitespace");

                return;
            }
        }
    }
}