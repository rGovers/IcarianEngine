using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics.Shapes;
using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics
{
    struct DispatchCollisionData
    {
        public uint IsTrigger;

        public uint BodyAddrA;
        public uint BodyAddrB;

        public Vector3 Normal;
        public float Depth;
    }

    public struct CollisionData
    {
        public Vector3 Normal;
        public float Depth;
    }

    public class PhysicsBody : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreatePhysicsBody(uint a_transformAddr, uint a_colliderAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyPhysicsBody(uint a_addr);

        public PhysicsBodyDef PhysicsBodyDef
        {
            get
            {
                return Def as PhysicsBodyDef;
            }
        }

        static ConcurrentDictionary<uint, PhysicsBody> s_bodies = new ConcurrentDictionary<uint, PhysicsBody>();

        bool           m_disposed = false;

        CollisionShape m_collisionShape = null;

        internal uint InternalAddr
        {
            get;
            set;
        }

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public CollisionShape CollisionShape
        {
            get
            {
                return m_collisionShape;
            }
            set
            {
                CollisionShapeSet(m_collisionShape, value);

                m_collisionShape = value;

                if (s_bodies.ContainsKey(InternalAddr))
                {
                    s_bodies[InternalAddr] = this;
                }
                else
                {
                    s_bodies.TryAdd(InternalAddr, this);
                }
            }
        }

        public PhysicsBody()
        {
            InternalAddr = uint.MaxValue;
        }        

        internal static PhysicsBody GetBody(uint a_addr)
        {
            if (s_bodies.ContainsKey(a_addr))
            {
                return s_bodies[a_addr];
            }

            return null;
        }

        public override void Init()
        {
            base.Init();

            PhysicsBodyDef def = PhysicsBodyDef;
            if (def != null)
            {
                CollisionShape = AssetLibrary.GetCollisionShape(def.CollisionShape);
            }
        }

        protected internal virtual void CollisionShapeSet(CollisionShape a_oldShape, CollisionShape a_newShape)
        {
            if (InternalAddr != uint.MaxValue)
            {
                DestroyPhysicsBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                InternalAddr = CreatePhysicsBody(Transform.InternalAddr, a_newShape.InternalAddr);
            }
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    CollisionShape = null;

                    m_disposed = true;
                }
                else
                {
                    Logger.IcarianWarning("PhysicsBody Failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple PhysicsBody Dispose");
            }
        }

        ~PhysicsBody()
        {
            Dispose(false);
        }

        static void OnCollisionEnter(DispatchCollisionData a_data)
        {
            if (!s_bodies.ContainsKey(a_data.BodyAddrA) && !s_bodies.ContainsKey(a_data.BodyAddrB))
            {
                Logger.IcarianError("Bad Collision Enter dispatch");

                return;
            }

            PhysicsBody bodyA = s_bodies[a_data.BodyAddrA];
            PhysicsBody bodyB = s_bodies[a_data.BodyAddrB];

            if (a_data.IsTrigger == 0)
            {
                if (bodyA is RigidBody rBodyA)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = a_data.Normal,
                        Depth = a_data.Depth  
                    };

                    rBodyA.OnCollisionEnter(bodyB, data);
                }

                if (bodyB is RigidBody rBodyB)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = -a_data.Normal,
                        Depth = a_data.Depth
                    };

                    rBodyB.OnCollisionEnter(bodyA, data);
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA)
                {
                    tBodyA.OnTriggerEnter(bodyB);
                }

                if (bodyB is TriggerBody tBodyB)
                {
                    tBodyB.OnTriggerEnter(bodyA);
                }
            }
        }
        static void OnCollisionStay(DispatchCollisionData a_data)
        {
            if (!s_bodies.ContainsKey(a_data.BodyAddrA) && !s_bodies.ContainsKey(a_data.BodyAddrB))
            {
                Logger.IcarianError("Bad Collision Stay dispatch");

                return;
            }

            PhysicsBody bodyA = s_bodies[a_data.BodyAddrA];
            PhysicsBody bodyB = s_bodies[a_data.BodyAddrB];

            if (a_data.IsTrigger == 0)
            {
                if (bodyA is RigidBody rBodyA)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = a_data.Normal,
                        Depth = a_data.Depth
                    };

                    rBodyA.OnCollisionStay(bodyB, data);
                }

                if (bodyB is RigidBody rBodyB)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = -a_data.Normal,
                        Depth = a_data.Depth
                    };

                    rBodyB.OnCollisionStay(bodyA, data);
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA)
                {
                    tBodyA.OnTriggerStay(bodyB);
                }

                if (bodyB is TriggerBody tBodyB)
                {
                    tBodyB.OnTriggerStay(bodyA);
                }
            }
        }
        static void OnCollisionExit(DispatchCollisionData a_data)
        {
            if (!s_bodies.ContainsKey(a_data.BodyAddrA) && !s_bodies.ContainsKey(a_data.BodyAddrB))
            {
                Logger.IcarianError("Bad Collision Exit dispatch");

                return;
            }

            PhysicsBody bodyA = s_bodies[a_data.BodyAddrA];
            PhysicsBody bodyB = s_bodies[a_data.BodyAddrB];

            if (a_data.IsTrigger == 0)
            {
                if (bodyA is RigidBody rBodyA)
                {
                    rBodyA.OnCollisionExit(bodyB);
                }

                if (bodyB is RigidBody rBodyB)
                {
                    rBodyB.OnCollisionExit(bodyA);
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA)
                {
                    tBodyA.OnTriggerExit(bodyB);
                }

                if (bodyB is TriggerBody tBodyB)
                {
                    tBodyB.OnTriggerExit(bodyA);
                }
            }
        }
    }
}