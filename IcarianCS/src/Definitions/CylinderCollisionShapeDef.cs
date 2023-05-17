using IcarianEngine.Physics.Shapes;

namespace IcarianEngine.Definitions
{
    public class CylinderCollisionShapeDef : CollisionShapeDef
    {
        public float Radius = 0.5f;
        public float Height = 1.0f;

        public CylinderCollisionShapeDef()
        {
            CollisionShapeType = typeof(CylinderCollisionShape);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (CollisionShapeType != typeof(CylinderCollisionShape) && !CollisionShapeType.IsSubclassOf(typeof(CylinderCollisionShape)))
            {
                Logger.IcarianError($"CylinderCollisionShapeDef Invalid CollisionShapeType: {CollisionShapeType}");

                return;
            }
        }
    }
}