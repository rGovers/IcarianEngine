// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

#include "EngineCylinderCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_CYLINDERCOLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics.Shapes
{
    public class CylinderCollisionShape : CollisionShape, IDestroy
    {
        /// <summary>
        /// The Defintion used to create the CylinderCollisionShape
        /// </summary>
        public CylinderCollisionShapeDef CylinderDef
        {
            get
            {
                return Def as CylinderCollisionShapeDef;
            }
        }

        /// <summary>
        /// The height of the CylinderCollisionShape
        /// </summary>
        public float Height
        {
            get
            {
                return CylinderCollisionShapeInterop.GetHeight(InternalAddr);
            }
        }
        /// <summary>
        /// The radius of the CylinderCollisionShape
        /// </summary>
        public float Radius
        {
            get
            {
                return CylinderCollisionShapeInterop.GetRadius(InternalAddr);
            }
        }

        /// <summary>
        /// Whether the CylinderCollisionShape is Disposed
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        CylinderCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }
        /// <summary>
        /// Creates an Instance of <see cref="IcarianEngine.Physics.Shapes.CylinderCollisionShape"/>
        /// </summary>
        /// <param name="a_height">The height of the CylinderCollisionShape</param>
        /// <param name="a_radius">The radius of the CylinderCollisionShape</param>
        public CylinderCollisionShape(float a_height, float a_radius)
        {
            InternalAddr = CylinderCollisionShapeInterop.CreateCylinder(a_height, a_radius);
        }

        internal override void Init()
        {
            CylinderCollisionShapeDef def = CylinderDef;

            if (def != null)
            {
                InternalAddr = CylinderCollisionShapeInterop.CreateCylinder(def.Height, def.Radius);
            }
            else
            {
                InternalAddr = CylinderCollisionShapeInterop.CreateCylinder(1.0f, 0.5f);
            }
        }

        /// <summary>
        /// Disposes of the CylinderCollisionShape
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the CylinderCollisionShape is Disposed
        /// </summary>
        /// <param name="a_disposing">Whether the CylinderCollisionShape is being Disposed or Finalized</param>
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
                    Logger.IcarianWarning("CylinderCollisionShape Failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple CylinderCollisionShape Dispose");
            }
        }
        ~CylinderCollisionShape()
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