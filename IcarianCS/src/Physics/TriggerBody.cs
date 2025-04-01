// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

        protected internal override void RebuildBody()
        {
            if (InternalAddr != uint.MaxValue)
            {
                PhysicsBodyInterop.DestroyPhysicsBody(InternalAddr);
                InternalAddr = uint.MaxValue;
            }

            CollisionShape shape = CollisionShape;
            if (shape != null)
            {
                InternalAddr = TriggerBodyInterop.CreateTriggerBody(Transform.InternalAddr, shape.InternalAddr);
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