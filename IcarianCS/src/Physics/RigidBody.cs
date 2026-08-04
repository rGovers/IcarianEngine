// Icarian Engine - C# Game Engine
//
// License at end of file.

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

        uint                 m_objectLayer;
        float                m_mass;
        float                m_gravityFactor;
        RigidBodyCastQuality m_castQuality;
        byte                 m_constraints;

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
        /// The <see cref="IcarianEngine.Definitions.RigidBodyDef" /> used to create the RigidBody
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
                if (m_mass == value)
                {
                    return;
                }

                m_mass = value;

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
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
                if (m_gravityFactor == value)
                {
                    return;
                }

                m_gravityFactor = value;

                if (InternalAddr != uint.MaxValue)
                {
                    RigidBodyInterop.SetGravityFactor(InternalAddr, m_gravityFactor);
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
            set
            {
                if (m_objectLayer == value)
                {
                    return;
                }

                m_objectLayer = value;

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
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
        /// The RigidBody collision cast quality
        /// </summary>
        public RigidBodyCastQuality CastQuality
        {
            get
            {
                return m_castQuality;
            }
            set
            {
                if (m_castQuality == value)
                {
                    return;
                }

                m_castQuality = value;

                if (InternalAddr != uint.MaxValue)
                {
                    RigidBodyInterop.SetCastQuality(InternalAddr, (uint)m_castQuality);
                }
            }
        }

        /// <summary>
        /// Whether to constain tranlation forces applied on the X axis
        /// </summary>
        public bool TranslateXConstained
        {
            get
            {
                return (m_constraints & (0b1U << (int)RigidBodyConstraints.TranslateX)) != 0;
            }
            set
            {
                bool oldValue = (m_constraints & (0b1U << (int)RigidBodyConstraints.TranslateX)) != 0;
                if (value == oldValue)
                {
                    return;
                }

                unchecked
                {
                    if (value)
                    {
                        m_constraints |= (byte)(0b1U << (int)RigidBodyConstraints.TranslateX);
                    }
                    else
                    {
                        m_constraints &= (byte)~(0b1U << (int)RigidBodyConstraints.TranslateX);
                    }
                }

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
            }
        }
        /// <summary>
        /// Whether to constain tranlation forces applied on the Y axis
        /// </summary>
        public bool TranslateYConstrained
        {
            get
            {
                return (m_constraints & (0b1U << (int)RigidBodyConstraints.TranslateY)) != 0;
            }
            set
            {
                bool oldValue = (m_constraints & (0b1U << (int)RigidBodyConstraints.TranslateY)) != 0;
                if (value == oldValue)
                {
                    return;
                }

                unchecked
                {
                    if (value)
                    {
                        m_constraints |= (byte)(0b1U << (int)RigidBodyConstraints.TranslateY);
                    }
                    else
                    {
                        m_constraints &= (byte)~(0b1U << (int)RigidBodyConstraints.TranslateY);
                    }
                }

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
            }
        }
        /// <summary>
        /// Whether to constain tranlation forces applied on the Z axis
        /// </summary>
        public bool TranslateZConstrained
        {
            get
            {
                return (m_constraints & (0b1U << (int)RigidBodyConstraints.TranslateZ)) != 0;
            }
            set
            {
                bool oldValue = (m_constraints & (0b1U << (int)RigidBodyConstraints.TranslateZ)) != 0;
                if (value == oldValue)
                {
                    return;
                }

                unchecked
                {
                    if (value)
                    {
                        m_constraints |= (byte)(0b1U << (int)RigidBodyConstraints.TranslateZ);
                    }
                    else
                    {
                        m_constraints &= (byte)~(0b1U << (int)RigidBodyConstraints.TranslateZ);
                    }
                }

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
            }
        }

        /// <summary>
        /// Whether to constain rotation forces applied on the X axis
        /// </summary>
        public bool RotateXConstrained
        {
            get
            {
                return (m_constraints & (0b1U << (int)RigidBodyConstraints.RotateX)) != 0;
            }
            set
            {
                bool oldValue = (m_constraints & (0b1U << (int)RigidBodyConstraints.RotateX)) != 0;
                if (value == oldValue)
                {
                    return;
                }

                unchecked
                {
                    if (value)
                    {
                        m_constraints |= (byte)(0b1U << (int)RigidBodyConstraints.RotateX);
                    }
                    else
                    {
                        m_constraints &= (byte)~(0b1U << (int)RigidBodyConstraints.RotateX);
                    }
                }

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
            }
        }
        /// <summary>
        /// Whether to constain rotation forces applied on the Y axis
        /// </summary>
        public bool RotateYConstrained
        {
            get
            {
                return (m_constraints & (0b1U << (int)RigidBodyConstraints.RotateY)) != 0;
            }
            set
            {
                bool oldValue = (m_constraints & (0b1U << (int)RigidBodyConstraints.RotateY)) != 0;
                if (value == oldValue)
                {
                    return;
                }

                unchecked
                {
                    if (value)
                    {
                        m_constraints |= (byte)(0b1U << (int)RigidBodyConstraints.RotateY);
                    }
                    else
                    {
                        m_constraints &= (byte)~(0b1U << (int)RigidBodyConstraints.RotateY);
                    }
                }

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
            }
        }
        /// <summary>
        /// Whether to constain rotation forces applied on the Z axis
        /// </summary>
        public bool RotateZConstrained
        {
            get
            {
                return (m_constraints & (0b1U << (int)RigidBodyConstraints.RotateZ)) != 0;
            }
            set
            {
                bool oldValue = (m_constraints & (0b1U << (int)RigidBodyConstraints.RotateZ)) != 0;
                if (value == oldValue)
                {
                    return;
                }

                unchecked
                {
                    if (value)
                    {
                        m_constraints |= (byte)(0b1U << (int)RigidBodyConstraints.RotateZ);
                    }
                    else
                    {
                        m_constraints &= (byte)~(0b1U << (int)RigidBodyConstraints.RotateZ);
                    }
                }

                RebuildBody();
                if (InternalAddr != uint.MaxValue)
                {
                    SetBody(InternalAddr, this);
                }
            }
        }

        public RigidBody()
        {
            m_objectLayer = 0;
            m_mass = 10.0f;
            m_gravityFactor = 1.0f;
            m_castQuality = RigidBodyCastQuality.Discrete;
            m_constraints = 0;
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
                Logger.IcarianWarning("Adding force to unintialised RigidBody");

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
                Logger.IcarianWarning("Adding torque to unintialised RigidBody");

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

        protected internal override void RebuildBody()
        {
            Vector3 vel = Vector3.Zero;
            Vector3 angVel = Vector3.Zero;
            if (InternalAddr != uint.MaxValue)
            {
                vel = RigidBodyInterop.GetVelocity(InternalAddr);
                angVel = RigidBodyInterop.GetAngularVelocity(InternalAddr);

                // Want to make sure the old body does not interact with the new body
                // There may still be interactions due to it being deferred for async purposes
                // May be some funkyness but should hopefully sort itself out
                // Physics is a headache as it is async and calls back to C#
                PhysicsBodyInterop.SetPosition(InternalAddr, new Vector3(float.MinValue));
                RigidBodyInterop.SetVelocity(InternalAddr, Vector3.Zero);
                PhysicsBodyInterop.DestroyPhysicsBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            CollisionShape shape = CollisionShape;
            if (shape != null)
            {
                InternalAddr = RigidBodyInterop.CreateRigidBody
                (
                    Transform.InternalAddr,
                    shape.InternalAddr,
                    m_objectLayer,
                    (uint)m_constraints,
                    m_mass
                );

                RigidBodyInterop.SetCastQuality(InternalAddr, (uint)m_castQuality);
                RigidBodyInterop.SetGravityFactor(InternalAddr, m_gravityFactor);
                RigidBodyInterop.SetVelocity(InternalAddr, vel);
                RigidBodyInterop.SetAngularVelocity(InternalAddr, angVel);
            }
        }
    };
}

// MIT License
//
// Copyright (c) 2026 River Govers
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
