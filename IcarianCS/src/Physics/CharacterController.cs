// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using IcarianEngine.Physics.Shapes;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineCharacterControllerInterop.h"
#include "InteropBinding.h"

ENGINE_CHARACTERCONTROLLER_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics
{
    public class CharacterController : Component, IDestroy
    {
        /// <summary>
        /// Adjust Velocity callback delegate
        /// </summary>
        /// <param name="a_controller">The CharacterController that initiated</param>
        /// <param name="a_body">A read-only <see cref="IcarianEngine.Physics.PhysicsBody" /> copy of the body</param>
        /// <param name="a_velocity">Output <see cref="IcarianEngine.Physics.PhysicsBody" /> velocity</param>
        /// <param name="a_angularVelocity">Output <see cref="IcarianEngine.Physics.PhysicsBody" /> angular velocity</param>
        public delegate void AdjustVelocityCallback(CharacterController a_controller, PhysicsBody a_body, out Vector3 a_velocity, out Vector3 a_angularVelocity);
        /// <summary>
        /// Contact Validate callback delegate
        /// </summary>
        /// <param name="a_controller">The CharacterController that initiated</param>
        /// <param name="a_body">The <see cref="IcarianEngine.Physics.PhysicsBody" /> to collide with</param>
        /// <returns>If the CharacterController can collide with the <see cref="IcarianEngine.Physics.PhysicsBody" /></returns>
        public delegate bool ContactValidateCallback(CharacterController a_controller, PhysicsBody a_body);
        /// <summary>
        /// Contact Add callback delegate
        /// </summary>
        /// <param name="a_controller">The CharacterController that initiated</param>
        /// <param name="a_body">The <see cref="IcarianEngine.Physics.PhysicsBody" /> that came into contact</param>
        /// <param name="a_position">The position of the contact</param>
        /// <param name="a_normal">The normal of the contact</param>
        public delegate void ContactAddCallback(CharacterController a_controller, PhysicsBody a_body, Vector3 a_position, Vector3 a_normal);
        /// <summary>
        /// Contact Solve callback delegate
        /// </summary>
        /// <param name="a_controller">The CharacterController to solve</param>
        /// <param name="a_body">The <see cref="IcarianEngine.Physics.PhysicsBody" /> being hit</param>
        /// <param name="a_position">The world position of the contact</param>
        /// <param name="a_normal">The world normal of the contact</param>
        /// <param name="a_contactVelocity">The velocity of the contact point</param>
        /// <param name="a_characterVelocity">The velocity of the character</param>
        /// <returns>The new velocity for the CharacterController</returns>
        public delegate Vector3 ContactSolveCallback(CharacterController a_controller, PhysicsBody a_body, Vector3 a_position, Vector3 a_normal, Vector3 a_contactVelocity, Vector3 a_characterVelocity);

        static CharacterController[] s_characters;

        bool           m_disposed = false;
        uint           m_internalAddr = uint.MaxValue;
        float          m_mass = 100.0f;
        float          m_slopeAngle = 1.0f;
        Vector3        m_up = new Vector3(0.0f, -1.0f, 0.0f);
        CollisionShape m_shape = null;

        /// <summary>
        /// Callback used to adjust velocity of the CharacterController
        /// </summary>
        public AdjustVelocityCallback OnAdjustVelocityCallback;
        /// <summary>
        /// Callback used to validate contact with <see cref="IcarianEngine.Physics.PhysicsBody" />(s)
        /// </summary>
        public ContactValidateCallback OnContactValidateCallback;
        /// <summary>
        /// Callback when a contact is added to the CharacterController
        /// </summary>
        public ContactAddCallback OnContactAddCallback;
        /// <summary>
        /// Callback used to solve collisions with the CharacterController
        /// </summary>
        public ContactSolveCallback OnContactSolveCallback;

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
        /// The Definition used to create the CharacterController
        /// </summary>
        public CharacterControllerDef CharacterControllerDef
        {
            get
            {
                return Def as CharacterControllerDef;
            }
        }

        /// <summary>
        /// Whether or not the CharacterController has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        /// <summary>
        /// The mass of the CharacterController
        /// </summary>
        public float Mass
        {
            get
            {
                return m_mass;
            }
            set
            {
                m_mass = value;

                RegenController();
            }
        }

        /// <summary>
        /// The maximum slope angle for the CharacterController
        /// </summary>
        public float SlopeAngle
        {
            get
            {
                return m_slopeAngle;
            }
            set
            {
                m_slopeAngle = value;

                RegenController();
            }
        }

        /// <summary>
        /// The up vector for the CharacterController
        /// </summary>
        public Vector3 Up
        {
            get
            {
                return m_up;
            }
            set
            {
                m_up = value;

                RegenController();
            }
        }

        /// <summary>
        /// The collider for the CharacterController
        /// </summary>
        public CollisionShape CollisionShape
        {
            get
            {
                return m_shape;
            }
            set
            {
                m_shape = value;

                RegenController();
            }
        }

        /// <summary>
        /// The velocity of the CharacterController
        /// </summary>
        public Vector3 Velocity
        {
            get
            {
                if (m_internalAddr == uint.MaxValue)
                {
                    return Vector3.Zero;
                }

                return CharacterControllerInterop.GetVelocity(m_internalAddr);
            }
            set
            {
                if (m_internalAddr == uint.MaxValue)
                {
                    return;
                }

                CharacterControllerInterop.SetVelocity(m_internalAddr, value);
            }
        }

        void RegenController()
        {
            if (m_internalAddr != uint.MaxValue)
            {
                CharacterControllerInterop.DestroyCharacter(m_internalAddr);

                if (s_characters != null)
                {
                    s_characters[m_internalAddr] = null;
                }

                m_internalAddr = uint.MaxValue;
            }

            if (m_shape == null)
            {
                return;   
            }

            uint transformAddr = Transform.InternalAddr;
            uint shapeAddr = m_shape.InternalAddr;

            m_internalAddr = CharacterControllerInterop.CreateCharacter(transformAddr, shapeAddr, m_up, m_slopeAngle, m_mass);

            if (m_internalAddr == uint.MaxValue)
            {
                return;
            }

            if (s_characters == null)
            {
                s_characters = new CharacterController[(m_internalAddr + 1) << 1];
            }
            else if (m_internalAddr >= s_characters.Length)
            {
                CharacterController[] newChars = new CharacterController[(m_internalAddr + 1) << 1];
                Array.Copy(s_characters, newChars, s_characters.Length);
                
                s_characters = newChars;
            }

            s_characters[m_internalAddr] = this;
        }

        internal static CharacterController GetCharacter(uint a_addr)
        {
            return s_characters[a_addr];
        }

        /// <summary>
        /// Called when the CharacterController is created
        /// </summary>
        public override void Init()
        {
            CharacterControllerDef def = CharacterControllerDef;
            if (def != null)
            {
                m_mass = def.Mass;
                m_slopeAngle = def.SlopeAngle;
                m_up = def.Up;
                m_shape = AssetLibrary.GetCollisionShape(def.CollisionShape);

                RegenController();
            }
        }

        static void OnAdjustVelocity(uint a_controllerAddr, uint a_bodyAddr, IntPtr a_velocity, IntPtr a_angularVelocity)
        {
            CharacterController character = GetCharacter(a_controllerAddr);
            if (character == null)
            {
                return;
            }

            if (character.OnAdjustVelocityCallback == null)
            {
                return;
            }

            PhysicsBody body = PhysicsBody.GetBody(a_bodyAddr);
            if (body == null)
            {
                return;
            }

            Vector3 vel;
            Vector3 angularVel;
            character.OnAdjustVelocityCallback(character, body, out vel, out angularVel);

            if (a_velocity != IntPtr.Zero)
            {
                Marshal.StructureToPtr(vel, a_velocity, false);
            }

            if (a_angularVelocity != IntPtr.Zero)
            {
                Marshal.StructureToPtr(angularVel, a_angularVelocity, false);
            }
        }
        static void OnContactValidate(uint a_controllerAddr, uint a_bodyAddr, IntPtr a_state)
        {
            CharacterController character = GetCharacter(a_controllerAddr);
            if (character == null)
            {
                return;
            }

            if (character.OnContactValidateCallback == null)
            {
                return;
            }

            PhysicsBody body = PhysicsBody.GetBody(a_bodyAddr);
            if (body == null)
            {
                return;
            }

            bool state = character.OnContactValidateCallback(character, body);
            if (a_state != IntPtr.Zero)
            {
                if (state)
                {
                    Marshal.WriteInt32(a_state, 1);
                }
                else
                {
                    Marshal.WriteInt32(a_state, 0);
                }
            }
        }
        static void OnContactAdd(uint a_controllerAddr, uint a_bodyAddr, Vector3 a_position, Vector3 a_normal)
        {
            CharacterController character = GetCharacter(a_controllerAddr);
            if (character == null)
            {
                return;
            }

            if (character.OnContactAddCallback == null)
            {
                return;
            }

            PhysicsBody body = PhysicsBody.GetBody(a_bodyAddr);
            if (body == null)
            {
                return;
            }

            character.OnContactAddCallback(character, body, a_position, a_normal);
        }
        static void OnContactSolve(uint a_controllerAddr, uint a_bodyAddr, Vector3 a_position, Vector3 a_normal, Vector3 a_contactVelocity, Vector3 a_characterVelocity, IntPtr a_outVelocity)
        {
            CharacterController character = GetCharacter(a_controllerAddr);
            if (character == null)
            {
                return;
            }

            if (character.OnContactSolveCallback == null)
            {
                return;
            }

            PhysicsBody body = PhysicsBody.GetBody(a_bodyAddr);
            if (body == null)
            {
                return;
            }

            Vector3 velocity = character.OnContactSolveCallback(character, body, a_position, a_normal, a_contactVelocity, a_characterVelocity);

            if (a_outVelocity != IntPtr.Zero)
            {
                Marshal.StructureToPtr(velocity, a_outVelocity, false);
            }
        }

        /// <summary>
        /// Disposes of the CharacterController
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the CharacterController is being Disposed/Finalised
        /// </summary
        /// <param name="a_disposing">Whether the CharacterController is being Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    CharacterControllerInterop.DestroyCharacter(m_internalAddr);

                    m_disposed = true;
                }
                else
                {
                    Logger.IcarianWarning("CharacterController Failed to Dispose");
                }

                m_internalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple CharacterController Dispose");
            }
        }
        ~CharacterController()
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