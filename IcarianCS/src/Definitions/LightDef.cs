using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class LightDef : ComponentDef
    {
        [EditorTooltip("Used to determine if it will be rendered by a camera in a matching layer. Binary bit based.")]
        public uint RenderLayer = 0b1;
        [EditorTooltip("Used to determine light color")]
        public Color Color = Color.White;
        [EditorTooltip("Used to determine light strength")]
        public float Intensity = 10.0f;

        public LightDef()
        {
            ComponentType = typeof(Light);
        }
    }
}