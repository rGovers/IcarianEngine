using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class RigidBodyDef : PhysicsBodyDef
    {
        public float Mass = 10.0f;

        public RigidBodyDef()
        {
            ComponentType = typeof(RigidBody);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(RigidBodyDef) && !ComponentType.IsSubclassOf(typeof(RigidBody)))
            {
                Logger.IcarianError($"Rigid Body Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}