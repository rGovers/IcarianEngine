using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class RendererDef : ComponentDef
    {
        [EditorTooltip("The material to use for rendering.")]
        public MaterialDef MaterialDef = null;

        public RendererDef()
        {
            ComponentType = typeof(Renderer);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (!ComponentType.IsSubclassOf(typeof(Renderer)))
            {
                Logger.IcarianError($"RendererDef {DefName} Invalid ComponentType: {ComponentType}");

                return;
            }

            if (MaterialDef == null)
            {
                Logger.IcarianWarning($"RendererDef {DefName} null material");
            }
        }
    }
}