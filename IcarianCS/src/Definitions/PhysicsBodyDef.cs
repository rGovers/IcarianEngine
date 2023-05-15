using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class PhysicsBodyDef : ComponentDef
    {
        public CollisionShapeDef CollisionShape;

        public PhysicsBodyDef()
        {
            ComponentType = typeof(PhysicsBody);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(PhysicsBody) || !ComponentType.IsSubclassOf(typeof(PhysicsBody)))
            {
                Logger.IcarianError($"Physics Body Def Invalid ComponentType: {ComponentType}");

                return;
            }

            if (CollisionShape == null)
            {
                Logger.IcarianWarning($"Physics Body Def null CollisionShape");

                return;
            }
        }
    };
}