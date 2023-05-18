using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class TriggerBodyDef : PhysicsBodyDef
    {
        public TriggerBodyDef()
        {
            ComponentType = typeof(TriggerBody);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(TriggerBody) && !ComponentType.IsSubclassOf(typeof(TriggerBody)))
            {
                Logger.IcarianError($"Trigger Body Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}