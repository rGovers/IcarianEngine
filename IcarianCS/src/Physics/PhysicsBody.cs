// Icarian Engine - C# Game Engine
// 
// License at end of file.

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
                if (m_collisionShape != value)
                {
                    m_collisionShape = value;

                    RebuildBody();
                    if (m_internalAddr != uint.MaxValue)
                    {
                        SetBody(m_internalAddr, this);
                    }
                }
            }
        }

        internal static PhysicsBody GetBody(uint a_addr)
        {
            if (s_bodies.ContainsKey(a_addr))
            {
                PhysicsBody body = s_bodies[a_addr];

                // May be a rebuilt body and a lingering call so we need to check
                if (body.m_internalAddr == a_addr)
                {
                    return body;
                }
            }

            return null;
        }
        internal static void SetBody(uint a_addr, PhysicsBody a_body)
        {
            if (s_bodies.ContainsKey(a_addr))
            {
                s_bodies[a_addr] = a_body;
            }
            else
            {
                s_bodies.TryAdd(a_addr, a_body);
            }
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

        protected internal virtual void RebuildBody()
        {
            if (m_internalAddr != uint.MaxValue)
            {
                PhysicsBodyInterop.DestroyPhysicsBody(m_internalAddr);

                m_internalAddr = uint.MaxValue;
            }

            if (m_collisionShape != null)
            {
                m_internalAddr = PhysicsBodyInterop.CreatePhysicsBody(Transform.InternalAddr, m_collisionShape.InternalAddr);
            }
        }

        /// <summary>
        /// Sets the position of the PhysicsBody
        /// </summary>
        /// <param name="a_pos">The position to set to</param>
        public void SetPosition(Vector3 a_pos)
        {
            PhysicsBodyInterop.SetPosition(m_internalAddr, a_pos);

            Transform.Translation = a_pos;
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

            Transform.Rotation = a_rotation;
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
            // Because of defered deletion there can be some lingering references should be resolved at the end of the update
            // But we need to return as they have been "deleted"
            // Also my dumbass did an and instead of or
            if (!s_bodies.ContainsKey(a_data.BodyAddrA) || !s_bodies.ContainsKey(a_data.BodyAddrB))
            {
                return;
            }

            PhysicsBody bodyA = s_bodies[a_data.BodyAddrA];
            PhysicsBody bodyB = s_bodies[a_data.BodyAddrB];
            if (bodyA == null || bodyB == null)
            {
                Logger.IcarianWarning("Null collision body");

                return;
            }

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

                    // Mostly an API safety thing to clean up user code want as minimal locks in user code as possible
                    // Redispatched the call as I kept shooting myself in the foot and decided to just fix the gun
                    // I may need to change the locks to NativeLock down the line as it may interfere with user code
                    lock (rBodyA)
                    {
                        // It is not impossible for it to be deleted so recheck once the lock is aquired
                        if (rBodyA != null && rBodyA.OnCollisionStartCallback != null)
                        {
                            rBodyA.OnCollisionStartCallback(bodyB, data);
                        }
                    }
                }

                if (bodyB is RigidBody rBodyB && rBodyB.OnCollisionStartCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Position = a_data.Position,
                        Normal = -a_data.Normal,
                        Depth = a_data.Depth
                    };

                    lock (rBodyB)
                    {
                        if (rBodyB != null && rBodyB.OnCollisionStartCallback != null)
                        {
                            rBodyB.OnCollisionStartCallback(bodyA, data);
                        }
                    }

                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA && tBodyA.OnTriggerStartCallback != null)
                {
                    lock (tBodyA)
                    {
                        if (tBodyA != null && tBodyA.OnTriggerStartCallback != null)
                        {
                            tBodyA.OnTriggerStartCallback(bodyB);
                        }
                    }
                }

                if (bodyB is TriggerBody tBodyB && tBodyB.OnTriggerStartCallback != null)
                {
                    lock (tBodyB)
                    {
                        if (tBodyB != null && tBodyB.OnTriggerStartCallback != null)
                        {
                            tBodyB.OnTriggerStartCallback(bodyA);
                        }
                    }
                }
            }
        }
        static void OnCollisionStay(CollisionDataBuffer a_data)
        {
            // Because of defered deletion there can be some lingering references should be resolved at the end of the update
            // But we need to return as they have been "deleted"
            // Also my dumbass did an and instead of or
            if (!s_bodies.ContainsKey(a_data.BodyAddrA) || !s_bodies.ContainsKey(a_data.BodyAddrB))
            {
                return;
            }

            PhysicsBody bodyA = s_bodies[a_data.BodyAddrA];
            PhysicsBody bodyB = s_bodies[a_data.BodyAddrB];
            if (bodyA == null || bodyB == null)
            {
                Logger.IcarianWarning("Null collision body");

                return;
            }

            if (a_data.IsTrigger == 0)
            {
                if (bodyA is RigidBody rBodyA && rBodyA.OnCollisionStayCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = a_data.Normal,
                        Depth = a_data.Depth
                    };

                    lock (rBodyA)
                    {
                        if (rBodyA != null && rBodyA.OnCollisionStayCallback != null)
                        {
                            rBodyA.OnCollisionStayCallback(bodyB, data);
                        }
                    }
                }

                if (bodyB is RigidBody rBodyB && rBodyB.OnCollisionStayCallback != null)
                {
                    CollisionData data = new CollisionData()
                    {
                        Normal = -a_data.Normal,
                        Depth = a_data.Depth
                    };

                    lock (rBodyB)
                    {
                        if (rBodyB != null && rBodyB.OnCollisionStayCallback != null)
                        {
                            rBodyB.OnCollisionStayCallback(bodyA, data);
                        }
                    }
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA && tBodyA.OnTriggerStayCallback != null)
                {
                    lock (tBodyA)
                    {
                        if (tBodyA != null && tBodyA.OnTriggerStayCallback != null)
                        {
                            tBodyA.OnTriggerStayCallback(bodyB);
                        }
                    }
                }

                if (bodyB is TriggerBody tBodyB && tBodyB.OnTriggerStayCallback != null)
                {
                    lock (tBodyB)
                    {
                        if (tBodyB != null && tBodyB.OnTriggerStayCallback != null)
                        {
                            tBodyB.OnTriggerStayCallback(bodyA);
                        }
                    }
                }
            }
        }
        static void OnCollisionExit(CollisionDataBuffer a_data)
        {
            // Because of defered deletion there can be some lingering references should be resolved at the end of the update
            // But we need to return as they have been "deleted"
            // Also my dumbass did an and instead of or
            if (!s_bodies.ContainsKey(a_data.BodyAddrA) || !s_bodies.ContainsKey(a_data.BodyAddrB))
            {
                return;
            }

            PhysicsBody bodyA = s_bodies[a_data.BodyAddrA];
            PhysicsBody bodyB = s_bodies[a_data.BodyAddrB];
            if (bodyA == null || bodyB == null)
            {
                Logger.IcarianWarning("Null collision body");

                return;
            }

            if (a_data.IsTrigger == 0)
            {
                if (bodyA is RigidBody rBodyA && rBodyA.OnCollisionEndCallback != null)
                {
                    lock (rBodyA)
                    {
                        if (rBodyA != null && rBodyA.OnCollisionEndCallback != null)
                        {
                            rBodyA.OnCollisionEndCallback(bodyB);
                        }
                    }
                }

                if (bodyB is RigidBody rBodyB && rBodyB.OnCollisionEndCallback != null)
                {
                    lock (rBodyB)
                    {
                        if (rBodyB != null && rBodyB.OnCollisionEndCallback != null)
                        {
                            rBodyB.OnCollisionEndCallback(bodyA);
                        }
                    }
                }
            }
            else
            {
                if (bodyA is TriggerBody tBodyA && tBodyA.OnTriggerEndCallback != null)
                {
                    lock (tBodyA)
                    {
                        if (tBodyA != null && tBodyA.OnTriggerEndCallback != null)
                        {
                            tBodyA.OnTriggerEndCallback(bodyB);
                        }
                    }
                }

                if (bodyB is TriggerBody tBodyB && tBodyB.OnTriggerEndCallback != null)
                {
                    lock (tBodyB)
                    {
                        if (tBodyB != null && tBodyB.OnTriggerEndCallback != null)
                        {
                            tBodyB.OnTriggerEndCallback(bodyA);
                        }
                    }
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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.