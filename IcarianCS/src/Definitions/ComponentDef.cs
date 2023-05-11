using System;

namespace IcarianEngine.Definitions
{
    public class ComponentDef : Def
    {
        public Type ComponentType = typeof(Component);

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType == null || !ComponentType.IsSubclassOf(typeof(Component)))
            {
                Logger.IcarianError($"Component Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}