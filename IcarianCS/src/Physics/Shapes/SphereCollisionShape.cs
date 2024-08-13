// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

#include "EngineSphereCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_SPHERECOLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics.Shapes
{
    public class SphereCollisionShape : CollisionShape, IDestroy
    {
        /// <summary>
        /// The Definition used to create the SphereCollisionShape
        /// </summary>
        public SphereCollisionShapeDef SphereDef
        {
            get
            {
                return Def as SphereCollisionShapeDef;
            }
        }

        /// <summary>
        /// Whether the SphereCollisionShape is Disposed
        /// <summary>
        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The radius of the SphereCollisionShape
        /// </summary>
        public float Radius
        {
            get
            {
                return SphereCollisionShapeInterop.GetRadius(InternalAddr);
            }
        }

        SphereCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }
        /// <summary>
        /// Create an Instance of <see cref="IcarianEngine.Physics.Shapes.SphereCollisionShape"/>
        /// </summary>
        /// <param name="a_radius">
        public SphereCollisionShape(float a_radius)
        {
            InternalAddr = SphereCollisionShapeInterop.CreateSphere(a_radius);
        }

        internal override void Init()
        {
            SphereCollisionShapeDef def = SphereDef;

            if (def != null)
            {
                InternalAddr = SphereCollisionShapeInterop.CreateSphere(def.Radius);
            }
            else
            {
                InternalAddr = SphereCollisionShapeInterop.CreateSphere(1.0f);
            }
        }

        /// <summary>
        /// Disposes of the SphereCollisionShape
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the SphereCollisionShape is being Disposed
        /// </summary>
        /// <param name="a_disposing">Whether the SphereCollisionShape is being Disposed or Finalized</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(InternalAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    CollisionShapeInterop.DestroyShape(InternalAddr);
                }
                else
                {
                    Logger.IcarianWarning("SphereCollisionShape Failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple SphereCollisionShape Dispose");
            }
        }

        ~SphereCollisionShape()
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