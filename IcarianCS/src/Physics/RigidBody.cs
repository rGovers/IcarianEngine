using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics.Shapes;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics
{
    public enum ForceMode : ushort
    {
        Force = 0,
        Acceleration = 1,
        Impulse = 2
    }

    public class RigidBody : PhysicsBody
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateRigidBody(uint a_transformAddr, uint a_colliderAddr, float a_mass);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint DestroyRigidBody(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector3 GetVelocity(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetVelocity(uint a_addr, Vector3 a_velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector3 GetAngularVelocity(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetAngularVelocity(uint a_addr, Vector3 a_velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddForce(uint a_addr, Vector3 a_force, uint a_mode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void AddTorque(uint a_addr, Vector3 a_torque, uint a_mode);

        public RigidBodyDef RigidBodyDef
        {
            get
            {
                return Def as RigidBodyDef;
            }
        }

        float m_mass = 10.0f;

        public float Mass
        {
            get
            {
                return m_mass;
            }
        }

        public Vector3 Velocity
        {
            get
            {
                return GetVelocity(InternalAddr);
            }
            set
            {
                SetVelocity(InternalAddr, value);
            }
        }

        public Vector3 AngularVelocity
        {
            get
            {
                return GetAngularVelocity(InternalAddr);
            }
            set
            {
                SetAngularVelocity(InternalAddr, value);
            }
        }

        public override void Init()
        {
            RigidBodyDef def = RigidBodyDef;
            if (def != null)
            {
                m_mass = def.Mass;
            }

            base.Init();
        }

        public void AddForce(Vector3 a_force, ForceMode a_forceMode = ForceMode.Force)
        {
            switch (a_forceMode)
            {
            case ForceMode.Acceleration:
            {
                AddForce(InternalAddr, a_force * Time.DeltaTime, (uint)ForceMode.Impulse);

                break;
            }
            default:
            {
                AddForce(InternalAddr, a_force, (uint)a_forceMode);

                break;
            }
            }
        }
        public void AddTorque(Vector3 a_torque, ForceMode a_forceMode = ForceMode.Force)
        {
            switch (a_forceMode)
            {
            case ForceMode.Acceleration:
            {
                AddTorque(InternalAddr, a_torque * Time.DeltaTime, (uint)ForceMode.Impulse);

                break;
            }
            default:
            {
                AddTorque(InternalAddr, a_torque, (uint)a_forceMode);

                break;
            }
            }
        }

        protected internal override void CollisionShapeSet(CollisionShape a_oldShape, CollisionShape a_newShape)
        {
            if (InternalAddr != uint.MaxValue)
            {
                DestroyRigidBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                InternalAddr = CreateRigidBody(Transform.InternalAddr, a_newShape.InternalAddr, m_mass);
            }
        }

        public virtual void OnCollisionEnter(PhysicsBody a_other, CollisionData a_data) { }
        public virtual void OnCollisionStay(PhysicsBody a_other, CollisionData a_data) { }
        public virtual void OnCollisionExit(PhysicsBody a_other) { }
    };
}