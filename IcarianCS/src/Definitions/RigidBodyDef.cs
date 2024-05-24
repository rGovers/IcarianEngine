using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class RigidBodyDef : PhysicsBodyDef
    {
        /// <summary>
        /// The ObjectLayer the <see cref="IcarianEngine.Physics.RigidBody" /> is on
        /// </summary>
        /// Ranges from 0-6
        [EditorTooltip("The ObjectLayer the Rigidbody is on")]
        public uint ObjectLayer = 0;
        /// <summary>
        /// The mass of the <see cref="IcarianEngine.Physics.RigidBody" />
        /// </summary>
        [EditorTooltip("The mass of the Rigidbody")]
        public float Mass = 10.0f;

        public RigidBodyDef()
        {
            ComponentType = typeof(RigidBody);
        }

        /// <summary>
        /// Called after the def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(RigidBody) && !ComponentType.IsSubclassOf(typeof(RigidBody)))
            {
                Logger.IcarianError($"RigidBodyDef Invalid ComponentType: {ComponentType}");

                return;
            }

            if (ObjectLayer > 6)
            {
                Logger.IcarianWarning($"RigidBodyDef out of range of moving ObjectLayers: {ObjectLayer}");

                return;
            }
        }
    }
}