using IcarianEngine.Rendering.Animation;

namespace IcarianEngine.Definitions
{
    public class SkinnedMeshRendererDef : RendererDef
    {
        /// <summary>
        /// Path relative to the project for the skeleton file to be used
        /// </summary>
        [EditorTooltip("Path relative to the project for the skeleton file to be used"), EditorPathString(new string[] { ".dae", ".fbx", ".glb", ".gltf" })]
        public string SkeletonPath;

        /// <summary>
        /// Path relative to the project for the model file to be used
        /// </summary>
        [EditorTooltip("Path relative to the project for the model file to be used"), EditorPathString(new string[] { ".dae", ".fbx", ".glb", ".gltf" })]
        public string ModelPath;

        /// <summary>
        /// The mesh index to load
        /// </summary>
        [EditorTooltip("The mesh index to load")]
        public byte Index;

        public SkinnedMeshRendererDef()
        {
            ComponentType = typeof(SkinnedMeshRenderer);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(SkinnedMeshRenderer) && !ComponentType.IsSubclassOf(typeof(SkinnedMeshRenderer)))
            {
                Logger.IcarianError($"SkinnedMeshRendererDef Invalid ComponentType: {ComponentType}");

                return;
            }

            if (string.IsNullOrWhiteSpace(ModelPath))
            {
                Logger.IcarianWarning("SkinnedMeshRendererDef Invalid ModelPath");
            }

            if (string.IsNullOrWhiteSpace(SkeletonPath))
            {
                Logger.IcarianWarning("SkinnedMeshRendererDef Invalid SkeletonPath");
            }
        }
    }
}