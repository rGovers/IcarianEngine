using IcarianEngine.Physics.Shapes;

namespace IcarianEngine.Definitions
{
    public class SphereCollisionShapeDef : CollisionShapeDef
    {
        public float Radius;

        public SphereCollisionShapeDef()
        {
            CollisionShapeType = typeof(SphereCollisionShape);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (CollisionShapeType != typeof(SphereCollisionShape) && !CollisionShapeType.IsSubclassOf(typeof(SphereCollisionShape)))
            {
                Logger.IcarianError($"SphereCollisionShapeDef Invalid CollisionShapeType: {CollisionShapeType}");

                return;
            }
        }
    }
}