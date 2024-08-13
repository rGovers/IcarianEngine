// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using System;
using System.Reflection;
using System.Runtime.CompilerServices;

#include "EngineCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_COLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION)

namespace IcarianEngine.Physics.Shapes
{
    public abstract class CollisionShape
    {
        CollisionShapeDef m_def;

        internal uint InternalAddr
        {
            get;
            set;
        }

        /// <summary>
        /// The def used to create the CollisionShape
        /// </summary>
        public CollisionShapeDef Def
        {
            get
            {
                return m_def;
            }
        }

        internal abstract void Init();

        /// <summary>
        /// Creates a CollisionShape from a def of type T
        /// <summary>
        /// <param name="a_def">The def to use to create the CollisionShape</param>
        /// <returns>The CollisionShape of type T. Null on failure.</returns>
        public static T FromDef<T>(CollisionShapeDef a_def) where T : CollisionShape
        {
            return FromDef(a_def) as T;
        }

        /// <summary>
        /// Creates a CollisionShape from a def
        /// <summary>
        /// <param name="a_def">The def to use to create the CollisionShape</param>
        /// <returns>The CollisionShape. Null on failure</returns>
        public static CollisionShape FromDef(CollisionShapeDef a_def)
        {
            CollisionShape shape = Activator.CreateInstance(a_def.CollisionShapeType, true) as CollisionShape;
            if (shape != null)
            {
                shape.m_def = a_def;
                shape.Init();
            }
            else
            {
                Logger.IcarianWarning($"Could not create CollisionShape from Def: {a_def}");
            }

            return shape;
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