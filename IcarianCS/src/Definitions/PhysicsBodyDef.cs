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

            if (ComponentType != typeof(PhysicsBody) && !ComponentType.IsSubclassOf(typeof(PhysicsBody)))
            {
                Logger.IcarianError($"PhysicsBodyDef {DefName} Invalid ComponentType: {ComponentType}");

                return;
            }

            if (CollisionShape == null)
            {
                Logger.IcarianWarning($"PhysicsBodyDef {DefName} null CollisionShape");

                return;
            }
        }
    };
}