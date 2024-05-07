using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class MeshRendererDef : RendererDef
    {
        /// <summary>
        /// Path relative to the project for the model file to be used
        /// </summary>
        [EditorTooltip("Path relative to the project for the model file to be used"), EditorPathString]
        public string ModelPath;        

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