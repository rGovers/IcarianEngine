using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class MeshRendererDef : RendererDef
    {
        [EditorTooltip("Path relative to the project for the model file to be used.")]
        public string ModelPath = null;        

        public MeshRendererDef()
        {
            ComponentType = typeof(MeshRenderer);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(MeshRenderer) && !ComponentType.IsSubclassOf(typeof(MeshRenderer)))
            {
                Logger.IcarianError($"MeshRendererDef Invalid ComponentType: {ComponentType}");

                return;
            }

            if (string.IsNullOrWhiteSpace(ModelPath))
            {
                Logger.IcarianWarning("MeshRendererDef Invalid ModelPath");
            }            
        }
    }
}