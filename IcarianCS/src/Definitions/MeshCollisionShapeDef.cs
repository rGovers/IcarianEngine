using IcarianEngine.Physics.Shapes;

namespace IcarianEngine.Definitions
{
    public class MeshCollisionShapeDef : CollisionShapeDef
    {
        /// <summary>
        /// Path relative to the project for the model file to be used
        /// </summary>
        [EditorTooltip("Path relative to the project for the model file to be used"), EditorPathString(new string[] { ".obj", ".dae", ".fbx", ".glb", ".gltf"})]
        public string MeshPath;

        public MeshCollisionShapeDef()
        {
            CollisionShapeType = typeof(MeshCollisionShape);
        }

        /// <summary>
        /// Called after final resolution of the Def
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (string.IsNullOrWhiteSpace(MeshPath))
            {
                Logger.IcarianWarning($"MeshCollisionShape {DefName} null MeshPath");

                return;
            }

            if (CollisionShapeType != typeof(MeshCollisionShape) && !CollisionShapeType.IsSubclassOf(typeof(MeshCollisionShape)))
            {
                Logger.IcarianWarning($"MeshCollisionShape {DefName} invalid CollisionShapeType: {CollisionShapeType}");

                return;
            }
        }
    }
}