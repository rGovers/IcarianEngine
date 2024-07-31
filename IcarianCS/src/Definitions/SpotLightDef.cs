using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class SpotLightDef : ShadowLightDef
    {
        /// <summary>
        /// The inner cutoff angle for the SpotLight
        /// </summary>
        [EditorTooltip("The inner cutoff angle for the SpotLight")]
        public float InnerCutoffAngle = 0.7f;
        /// <summary>
        /// The outer cutoff angle for the SpotLight
        /// </summary>
        [EditorTooltip("The outer cutoff angle for the SpotLight")]
        public float OuterCutoffAngle = 1.0f;
        /// <summary>
        /// The radius for the SpotLight
        /// </summary>
        [EditorTooltip("The radius for the SpotLight")]
        public float Radius = 10.0f;

        public SpotLightDef()
        {
            ComponentType = typeof(SpotLight);
        }

        /// <summary>
        /// Called after the Def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(SpotLight) && !ComponentType.IsSubclassOf(typeof(SpotLight)))
            {
                Logger.IcarianError($"SpotLightDef {DefName} Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}