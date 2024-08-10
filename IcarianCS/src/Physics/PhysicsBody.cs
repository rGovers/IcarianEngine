using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics.Shapes;
using System;
using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EnginePhysicsBodyInterop.h"
#include "EnginePhysicsBodyInteropStructures.h"
#include "InteropBinding.h"

ENGINE_PHYSICSBODY_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics
{
    public struct CollisionData
    {
        /// <summary>
        /// The position of the contact
        /// </summary>
        public Vector3 Position;
        /// <summary>
        /// The normal of the collision
        /// </summary>
        public Vector3 Normal;
        /// <summary>
        /// The depth of the collision
        /// </summary>
        public float Depth;
    }

    public class PhysicsBody : Component, IDestroy
    {
        static ConcurrentDictionary<uint, PhysicsBody> s_bodies = new ConcurrentDictionary<uint, PhysicsBody>();

        bool           m_disposed = false;

        CollisionShape m_collisionShape = null;

        uint           m_internalAddr = uint.MaxValue;

        internal uint InternalAddr
        {
            get
            {
                return m_internalAddr;
            }
            set
            {
                m_internalAddr = value;
            }
        }

        /// <summary>
        /// The Definition used to create the PhysicsBody
        /// </summary>
        public PhysicsBodyDef PhysicsBodyDef
        {
            get
            {
                return Def as PhysicsBodyDef;
            }
        }

        /// <summary>
        /// Whether the PhysicsBody had been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The collider the PhysicsBody uses
        /// </summary>
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

                if (s_bodies.ContainsKey(m_internalAddr))
                {
                    s_bodies[m_internalAddr] = this;
                }
                else
                {
                    s_bodies.TryAdd(m_internalAddr, this);
                }
            }
        }

        internal static PhysicsBody GetBody(uint a_addr)
        {
            if (s_bodies.ContainsKey(a_addr))
            {
                return s_bodies[a_addr];
            }

            return null;
        }

        /// <summary>
        /// Called when the PhysicsBody is created
        /// </summary>
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
            if (m_internalAddr != uint.MaxValue)
            {
                PhysicsBodyInterop.DestroyPhysicsBody(m_internalAddr);

                m_internalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                m_internalAddr = PhysicsBodyInterop.CreatePhysicsBody(Transform.InternalAddr, a_newShape.InternalAddr);
            }
        }

        /// <summary>
        /// Sets the position of the PhysicsBody
        /// </summary>
        /// <param name="a_pos">The position to set to</param>
        public void SetPosition(Vector3 a_pos)
        {
            PhysicsBodyInterop.SetPosition(m_internalAddr, a_pos);
        }
        /// <summary>
        /// Gets the position of the PhysicsBody
        /// </summary>
        public Vector3 GetPosition()
        {
            return PhysicsBodyInterop.GetPosition(m_internalAddr);
        }

        /// <summary>
        /// Sets the rotation of the PhysicsBody
        /// </summary>
        /// <param name="a_rotation">The rotation to set to</param>
        public void SetRotation(Quaternion a_rotation)
        {
            PhysicsBodyInterop.SetRotation(m_internalAddr, a_rotation);
        }
        /// <summary>
        /// Gets the rotation of the PhysicsBody
        /// </summary>
        public Quaternion GetRotation()
        {
            return PhysicsBodyInterop.GetRotation(m_internalAddr);
        }

        static void OnCollisionEnter(CollisionDataBuffer a_data)
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
                if (bodyA is RigidBody rBodyA && rBodyA.OnCollisionStartCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Position = a_data.Position,
                        Normal = a_data.Normal,
                        Depth = a_data.Depth  
                    };

                    rBodyA.OnCollisionStartCallback(bodyB, data);
                }

                if (bodyB is RigidBody rBodyB && rBodyB.OnCollisionStartCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Position = a_data.Position,
                        Normal = -a_data.Normal,
                        Depth = a_data.Depth
                    };

                    rBodyB.OnCollisionStartCallback(bodyA, data);
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA && tBodyA.OnTriggerStartCallback != null)
                {
                    tBodyA.OnTriggerStartCallback(bodyB);
                }

                if (bodyB is TriggerBody tBodyB && tBodyB.OnTriggerStartCallback != null)
                {
                    tBodyB.OnTriggerStartCallback(bodyA);
                }
            }
        }
        static void OnCollisionStay(CollisionDataBuffer a_data)
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
                if (bodyA is RigidBody rBodyA && rBodyA.OnCollisionStayCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = a_data.Normal,
                        Depth = a_data.Depth
                    };

                    rBodyA.OnCollisionStayCallback(bodyB, data);
                }

                if (bodyB is RigidBody rBodyB && rBodyB.OnCollisionStayCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = -a_data.Normal,
                        Depth = a_data.Depth
                    };

                    rBodyB.OnCollisionStayCallback(bodyA, data);
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA && tBodyA.OnTriggerStayCallback != null)
                {
                    tBodyA.OnTriggerStayCallback(bodyB);
                }

                if (bodyB is TriggerBody tBodyB && tBodyB.OnTriggerStayCallback != null)
                {
                    tBodyB.OnTriggerStayCallback(bodyA);
                }
            }
        }
        static void OnCollisionExit(CollisionDataBuffer a_data)
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
                if (bodyA is RigidBody rBodyA && rBodyA.OnCollisionEndCallback != null)
                {
                    rBodyA.OnCollisionEndCallback(bodyB);
                }

                if (bodyB is RigidBody rBodyB && rBodyB.OnCollisionEndCallback != null)
                {
                    rBodyB.OnCollisionEndCallback(bodyA);
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA && tBodyA.OnTriggerEndCallback != null)
                {
                    tBodyA.OnTriggerEndCallback(bodyB);
                }

                if (bodyB is TriggerBody tBodyB && tBodyB.OnTriggerEndCallback != null)
                {
                    tBodyB.OnTriggerEndCallback(bodyA);
                }
            }
        }

        /// <summary>
        /// Disposes of the PhysicsBody
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the PhysicsBody is being Disposed
        /// </summary
        /// <param name="a_disposing">Whether the PhysicsBody is being Disposed or Finalized</param>
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

                m_internalAddr = uint.MaxValue;
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
    }
}