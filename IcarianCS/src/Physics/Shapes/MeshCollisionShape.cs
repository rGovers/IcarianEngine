// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using System;
using System.Runtime.CompilerServices;

#include "EngineMeshCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_MESHCOLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics.Shapes
{
    public class MeshCollisionShape : CollisionShape, IDestroy
    {
        /// <summary>
        /// The Def used to create the MeshCollisionShape
        /// </summary>
        public MeshCollisionShapeDef MeshDef
        {
            get
            {
                return Def as MeshCollisionShapeDef;
            }
        }

        /// <summary>
        /// Whether the MeshCollisionShape has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        MeshCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        internal override void Init()
        {
            MeshCollisionShapeDef def = MeshDef;

            if (def != null)
            {
                string path = ModControl.GetAssetPath(def.MeshPath);
                if (string.IsNullOrEmpty(path))
                {
                    Logger.IcarianError("MeshCollisionShape failed to find mesh path: " + def.MeshPath);

                    return;
                }

                InternalAddr = MeshCollisionShapeInterop.CreateMesh(path);
            }
            else
            {
                Logger.IcarianError($"MeshCollisionShape null Def");
            }
        }
        
        /// <summary>
        /// Disposes of the MeshCollisionShape
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the MeshCollisionShape is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Whether it is being Disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if (InternalAddr != uint.MaxValue)
            {
                if (a_disposing)
                {
                    CollisionShapeInterop.DestroyShape(InternalAddr);
                }
                else
                {
                    Logger.IcarianWarning("MeshCollisionShape failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple MeshColllisionShape Dispose");
            }
        }
        ~MeshCollisionShape()
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