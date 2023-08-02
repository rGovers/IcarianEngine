using IcarianEngine.Rendering.Animation;
using System;

namespace IcarianEngine.Definitions
{
    public class AnimationControllerDef : Def
    {
        public Type ControllerType = typeof(AnimationController);

        public override void PostResolve()
        {
            base.PostResolve();

            if (ControllerType == null || !ControllerType.IsSubclassOf(typeof(AnimationController)))
            {
                Logger.IcarianError($"AnimationControllerDef Invalid ControllerType: {ControllerType}");

                return;
            }
        }
    }
}