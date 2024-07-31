using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class DirectionalLightDef : ShadowLightDef
    {
        public DirectionalLightDef()
        {
            ComponentType = typeof(DirectionalLight);
        }

        /// <summary>
        /// Called after the Def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(DirectionalLight) && !ComponentType.IsSubclassOf(typeof(DirectionalLight)))
            {
                Logger.IcarianError($"DirectionalLightDef {DefName} Invalid ComponentType: {ComponentType}");
            }
        }
    }
}