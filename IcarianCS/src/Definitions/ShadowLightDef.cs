using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class ShadowLightDef : LightDef
    {
        /// <summary>
        /// The depth bias for the ShadowLight
        /// </summary>
        [EditorTooltip("The depth bias for the ShadowLight")]
        public float ShadowBiasConstant;

        /// <summary>
        /// The depth slope for the ShadowLight
        /// </summary>
        [EditorTooltip("The depth slope for the ShadowLight")]
        public float ShadowBiasSlope;

        public ShadowLightDef()
        {
            ComponentType = typeof(ShadowLight);
        }

        /// <summary>
        /// Called after the Def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (!ComponentType.IsSubclassOf(typeof(ShadowLight)))
            {
                Logger.IcarianError($"ShadowLightDef {DefName} Invalid ComponentType: {ComponentType}");
            }
        }
    }
}