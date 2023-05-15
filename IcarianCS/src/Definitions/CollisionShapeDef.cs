using IcarianEngine.Physics.Shapes;
using System;

namespace IcarianEngine.Definitions
{
    public class CollisionShapeDef : Def
    {
        public Type CollisionShapeType = typeof(CollisionShape);

        public override void PostResolve()
        {
            if (CollisionShapeType != typeof(CollisionShape) && !CollisionShapeType.IsSubclassOf(typeof(CollisionShape)))
            {
                Logger.IcarianError($"Collision Shape Def Invalid CollisionShapeType: {CollisionShapeType}");

                return;
            }
        }
    };
}