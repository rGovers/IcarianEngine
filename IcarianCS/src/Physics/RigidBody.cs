using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics.Shapes;
using System.Runtime.CompilerServices;

#include "EngineRigidBodyInterop.h"
#include "EngineRigidBodyInteropStructures.h"
#include "InteropBinding.h"

ENGINE_RIGIDBODY_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics
{
    public class RigidBody : PhysicsBody
    {
        /// <summary>
        /// Collision callback delegate
        /// </summary>
        /// <param name="a_other">The other body involved in the collision</param>
        /// <param name="a_data">The data of the collision</param>
        public delegate void CollisionCallback(PhysicsBody a_other, CollisionData a_data);
        /// <summary>
        /// Collision end callback delegate
        /// </summary>
        /// <param name="a_other">The other body that collision ended with</param>
        public delegate void EndCollisionCallback(PhysicsBody a_other);

        uint  m_objectLayer = 0;
        float m_mass = 10.0f;
        float m_gravityFactor = 1.0f;

        /// <summary>
        /// Callback used for events on collision enter
        /// </summary>
        public CollisionCallback OnCollisionStartCallback;
        /// <summary>
        /// Callback used for events on collision stay.
        /// </summary>
        public CollisionCallback OnCollisionStayCallback;
        /// <summary>
        /// Callback used for events on collision exit
        /// </summary>
        public EndCollisionCallback OnCollisionEndCallback;

        /// <summary>
        /// The Definition used to create the RigidBody
        /// <summary>
        public RigidBodyDef RigidBodyDef
        {
            get
            {
                return Def as RigidBodyDef;
            }
        }

        /// <summary>
        /// The mass of the RigidBody
        /// </summary>
        public float Mass
        {
            get
            {
                return m_mass;
            }
            set
            {
                if (m_mass != value)
                {
                    m_mass = value;

                    CollisionShape shape = CollisionShape;

                    CollisionShapeSet(shape, shape);
                }
            }
        }

        /// <summary>
        /// The factor of Gravity applied to the RigidBody
        /// </summary>
        public float GravityFactor
        {
            get
            {
                return m_gravityFactor;
            }
            set
            {
                if (m_gravityFactor != value)
                {
                    m_gravityFactor = value;

                    if (InternalAddr != uint.MaxValue)
                    {
                        RigidBodyInterop.SetGravityFactor(InternalAddr, m_gravityFactor);
                    }
                }
            }
        }

        /// <summary>
        /// The layer the Rigidbody is on
        /// </summary>
        public uint ObjectLayer
        {
            get
            {
                return m_objectLayer;
            }
        }

        /// <summary>
        /// The velocity of the RigidBody
        /// </summary>
        public Vector3 Velocity
        {
            get
            {
                if (InternalAddr == uint.MaxValue)
                {
                    return Vector3.Zero;
                }

                return RigidBodyInterop.GetVelocity(InternalAddr);
            }
            set
            {
                if (InternalAddr == uint.MaxValue)
                {
                    Logger.IcarianWarning("Setting velocity of unintialised RigidBody");

                    return;
                }

                RigidBodyInterop.SetVelocity(InternalAddr, value);
            }
        }
        /// <summary>
        /// The angular velocity of the RigidBody
        /// </summary>
        public Vector3 AngularVelocity
        {
            get
            {
                if (InternalAddr == uint.MaxValue)
                {
                    return Vector3.Zero;
                }

                return RigidBodyInterop.GetAngularVelocity(InternalAddr);
            }
            set
            {
                if (InternalAddr == uint.MaxValue)
                {
                    Logger.IcarianWarning("Setting angular velocity of unintialised RigidBody");

                    return;
                }

                RigidBodyInterop.SetAngularVelocity(InternalAddr, value);
            }
        }

        /// <summary>
        /// Called when the RigidBody is created
        /// </summary>
        public override void Init()
        {
            RigidBodyDef def = RigidBodyDef;
            if (def != null)
            {
                m_mass = def.Mass;
                m_objectLayer = def.ObjectLayer;
            }

            base.Init();
        }

        /// <summary>
        /// Adds force to the RigidBody
        /// </summary>
        /// <param name="a_force">The force to apply to the body</param>
        /// <param name="a_forceMode">The force mode of the force</param>
        public void AddForce(Vector3 a_force, ForceMode a_forceMode = ForceMode.Force)
        {
            if (InternalAddr == uint.MaxValue)
            {
                Logger.IcarianWarning("Adding force to unitialised RigidBody");

                return;
            }

            switch (a_forceMode)
            {
            case ForceMode.Acceleration:
            {
                RigidBodyInterop.AddForce(InternalAddr, a_force * Time.FixedDeltaTime, (uint)ForceMode.Impulse);

                break;
            }
            default:
            {
                RigidBodyInterop.AddForce(InternalAddr, a_force, (uint)a_forceMode);

                break;
            }
            }
        }
        /// <summary>
        /// Adds torque to the RigidBody
        /// </summary>
        /// <param name="a_torque">The torque to apply to the body</param>
        /// <param name="a_forceMode">The mode of the torque</param>
        public void AddTorque(Vector3 a_torque, ForceMode a_forceMode = ForceMode.Force)
        {
            if (InternalAddr == uint.MaxValue)
            {
                Logger.IcarianWarning("Adding torque to unitialised RigidBody");

                return;
            }

            switch (a_forceMode)
            {
            case ForceMode.Acceleration:
            {
                RigidBodyInterop.AddTorque(InternalAddr, a_torque * Time.FixedDeltaTime, (uint)ForceMode.Impulse);

                break;
            }
            default:
            {
                RigidBodyInterop.AddTorque(InternalAddr, a_torque, (uint)a_forceMode);

                break;
            }
            }
        }

        protected internal override void CollisionShapeSet(CollisionShape a_oldShape, CollisionShape a_newShape)
        {
            Vector3 vel = Vector3.Zero;
            Vector3 angVel = Vector3.Zero;
            if (InternalAddr != uint.MaxValue)
            {
                vel = RigidBodyInterop.GetVelocity(InternalAddr);
                angVel = RigidBodyInterop.GetAngularVelocity(InternalAddr);

                PhysicsBodyInterop.DestroyPhysicsBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                InternalAddr = RigidBodyInterop.CreateRigidBody(Transform.InternalAddr, a_newShape.InternalAddr, m_objectLayer, m_mass);

                RigidBodyInterop.SetVelocity(InternalAddr, vel);
                RigidBodyInterop.SetAngularVelocity(InternalAddr, angVel);
            }
        }
    };
}