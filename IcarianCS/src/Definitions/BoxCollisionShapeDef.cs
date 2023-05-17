using IcarianEngine.Maths;
using IcarianEngine.Physics.Shapes;

namespace IcarianEngine.Definitions
{
    public class BoxCollisionShapeDef : CollisionShapeDef
    {
        public Vector3 Extents = Vector3.One;

        public BoxCollisionShapeDef()
        {
            CollisionShapeType = typeof(BoxCollisionShape);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (CollisionShapeType != typeof(BoxCollisionShape) && !CollisionShapeType.IsSubclassOf(typeof(BoxCollisionShape)))
            {
                Logger.IcarianError($"BoxCollisionShapeDef Invalid CollisionShapeType: {CollisionShapeType}");

                return;
            }
        }
    };
}