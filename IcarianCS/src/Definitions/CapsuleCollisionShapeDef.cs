using IcarianEngine.Physics.Shapes;

namespace IcarianEngine.Definitions
{
    public class CapsuleCollisionShapeDef : CollisionShapeDef
    {
        public float Radius = 0.5f;
        public float Height = 1.0f;

        public CapsuleCollisionShapeDef()
        {
            CollisionShapeType = typeof(CapsuleCollisionShape);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (CollisionShapeType != typeof(CapsuleCollisionShape) && !CollisionShapeType.IsSubclassOf(typeof(CapsuleCollisionShape)))
            {
                Logger.IcarianError($"CapsuleCollisionShapeDef Invalid CollisionShapeType: {CollisionShapeType}");

                return;
            }
        }
    }
}