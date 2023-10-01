using IcarianEngine.Definitions;
using IcarianEngine.Physics.Shapes;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics
{
    public class TriggerBody : PhysicsBody
    {
        // RREEEEEEEE!

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateTriggerBody(uint a_transformAddr, uint a_colliderAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyTriggerBody(uint a_addr);

        public delegate void TriggerCallback(PhysicsBody a_other);

        public TriggerCallback OnTriggerStartCallback;
        public TriggerCallback OnTriggerStayCallback;
        public TriggerCallback OnTriggerEndCallback;

        public TriggerBodyDef TriggerBodyDef
        {
            get
            {
                return Def as TriggerBodyDef;
            }
        }

        protected internal override void CollisionShapeSet(CollisionShape a_oldShape, CollisionShape a_newShape)
        {
            if (InternalAddr != uint.MaxValue)
            {
                DestroyTriggerBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                InternalAddr = CreateTriggerBody(Transform.InternalAddr, a_newShape.InternalAddr);
            }
        }
    }
}