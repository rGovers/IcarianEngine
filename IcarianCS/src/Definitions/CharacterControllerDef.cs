using IcarianEngine.Maths;
using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class CharacterControllerDef : ComponentDef
    {
        public float Mass = 100.0f;
        public float SlopeAngle = 1.0f;
        public Vector3 Up = new Vector3(0.0f, -1.0f, 0.0f);
        public CollisionShapeDef CollisionShape = null;

        public CharacterControllerDef()
        {
            ComponentType = typeof(CharacterController);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(CharacterController) && !ComponentType.IsSubclassOf(typeof(CharacterController)))
            {
                Logger.IcarianError($"CharacterControllerDef {DefName} Invalid ComponentType: {ComponentType}");

                return;
            }

            if (CollisionShape == null)
            {
                Logger.IcarianWarning($"CharacterControllerDef {DefName} null CollisionShape");

                return;
            }
        }
    }
}