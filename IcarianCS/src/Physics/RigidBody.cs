using IcarianEngine.Definitions;
using IcarianEngine.Physics.Shapes;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics
{
    public class RigidBody : PhysicsBody
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateRigidBody(uint a_transformAddr, uint a_colliderAddr, float a_mass);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint DestroyRigidBody(uint a_addr);

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

        public override void Init()
        {
            RigidBodyDef def = RigidBodyDef;
            if (def != null)
            {
                m_mass = def.Mass;
            }

            base.Init();
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
    };
}