// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class RigidBodyDef : PhysicsBodyDef
    {
        /// <summary>
        /// The ObjectLayer the <see cref="IcarianEngine.Physics.RigidBody" /> is on
        /// </summary>
        [EditorRange(0, 6), EditorTooltip("The ObjectLayer the Rigidbody is on")]
        public uint ObjectLayer = 0;
        /// <summary>
        /// The mass of the <see cref="IcarianEngine.Physics.RigidBody" />
        /// </summary>
        [EditorTooltip("The mass of the Rigidbody")]
        public float Mass = 10.0f;

        public RigidBodyDef()
        {
            ComponentType = typeof(RigidBody);
        }

        /// <summary>
        /// Called after the def is loaded to resolve any data
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(RigidBody) && !ComponentType.IsSubclassOf(typeof(RigidBody)))
            {
                Logger.IcarianError($"RigidBodyDef Invalid ComponentType: {ComponentType}");

                return;
            }

            if (ObjectLayer > 6)
            {
                Logger.IcarianWarning($"RigidBodyDef out of range of moving ObjectLayers: {ObjectLayer}");

                return;
            }
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