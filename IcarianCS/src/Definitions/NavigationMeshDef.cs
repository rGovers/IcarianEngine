using IcarianEngine.AI;

namespace IcarianEngine.Definitions
{
    public class NavigationMeshDef : ComponentDef
    {
        /// <summary>
        /// Path relative to the project for the model file to be used
        /// </summary>
        [EditorTooltip("Path relative to the project for the model file to be used"), EditorPathString(new string[] { ".obj", ".dae", ".fbx", ".glb", ".gltf"})]
        public string MeshPath;

        public NavigationMeshDef()
        {
            ComponentType = typeof(NavigationMesh);
        }

        /// <summary>
        /// Called after the NavigationMeshDef has been resolved
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(NavigationMesh) && ComponentType.IsSubclassOf(typeof(NavigationMesh)))
            {
                Logger.IcarianError($"NavigationMeshDef {DefName} invalid ComponentType: {ComponentType}");

                return;
            }

            if (string.IsNullOrWhiteSpace(MeshPath))
            {
                Logger.IcarianWarning($"NavigationMeshDef {DefName} null MeshPath");
            }
        }
    }
}