using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class MeshRendererDef : ComponentDef
    {
        [EditorTooltip("Path relative to the project for the model file to be used.")]
        public string ModelPath = null;

        [EditorTooltip("The material to use for rendering.")]
        public MaterialDef MaterialDef = null;

        public MeshRendererDef()
        {
            ComponentType = typeof(MeshRenderer);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(MeshRenderer) && !ComponentType.IsSubclassOf(typeof(MeshRenderer)))
            {
                Logger.IcarianError($"Mesh Renderer Def Invalid ComponentType: {ComponentType}");

                return;
            }

            if (string.IsNullOrWhiteSpace(ModelPath))
            {
                Logger.IcarianWarning("Mesh Renderer Def Invalid ModelPath");
            }

            if (MaterialDef == null)
            {
                Logger.IcarianWarning("Mesh Renderer Def Invalid Material");
            }
        }
    }
}