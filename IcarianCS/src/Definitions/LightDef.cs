using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class LightDef : ComponentDef
    {
        /// <summary>
        /// Used to determine if it will be rendered by a camera in a matching layer
        /// </summary>
        [EditorBitfield, EditorTooltip("Used to determine if it will be rendered by a camera in a matching layer.")]
        public uint RenderLayer = 0b1;
        /// <summary>
        /// Used to determine light color
        /// </summary>
        [EditorTooltip("Used to determine light color")]
        public Color Color = Color.White;
        /// <summary>
        /// Used to determine light strength
        /// </summary>
        [EditorTooltip("Used to determine light strength")]
        public float Intensity = 10.0f;

        public LightDef()
        {
            ComponentType = typeof(Light);
        }

        /// <summary>
        /// Called after the Def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (!ComponentType.IsSubclassOf(typeof(Light)))
            {
                Logger.IcarianError($"LightDef {DefName} Invalid ComponentType: {ComponentType}");
            }
        }
    }
}