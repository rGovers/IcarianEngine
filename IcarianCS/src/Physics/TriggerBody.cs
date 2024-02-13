using IcarianEngine.Definitions;
using IcarianEngine.Physics.Shapes;
using System.Runtime.CompilerServices;

#include "EngineTriggerBodyInterop.h"
#include "InteropBinding.h"

ENGINE_TRIGGERBODY_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics
{
    public class TriggerBody : PhysicsBody
    {
        // RREEEEEEEE!

        /// <summary>
        /// Trigger callback delegate
        /// </summary>
        /// <param name="a_other">The other body in the trigger</param>
        public delegate void TriggerCallback(PhysicsBody a_other);

        /// <sumamry>
        /// Callback used for events on trigger enter
        /// </summary>
        public TriggerCallback OnTriggerStartCallback;
        /// <summary>
        /// Callback used for events on trigger stay
        /// </summary>
        public TriggerCallback OnTriggerStayCallback;
        /// <summary>
        /// Callback used for events on trigger exit
        /// </summary>
        public TriggerCallback OnTriggerEndCallback;

        /// <summary>
        /// The Defintion used to create the TriggerBody
        /// </summary>
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
                PhysicsBodyInterop.DestroyPhysicsBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                InternalAddr = TriggerBodyInterop.CreateTriggerBody(Transform.InternalAddr, a_newShape.InternalAddr);
            }
        }
    }
}