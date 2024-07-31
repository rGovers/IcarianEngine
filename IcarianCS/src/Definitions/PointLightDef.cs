using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class PointLightDef : ShadowLightDef
    {
        /// <summary>
        /// The radius for the PointLight
        /// </summary>
        [EditorTooltip("The radius for the PointLight")]
        public float Radius = 1.0f;

        public PointLightDef()
        {
            ComponentType = typeof(PointLight);
        }

        /// <summary>
        /// Called after the Def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(PointLight) && !ComponentType.IsSubclassOf(typeof(PointLight)))
            {
                Logger.IcarianError($"PointLightDef {DefName} Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}