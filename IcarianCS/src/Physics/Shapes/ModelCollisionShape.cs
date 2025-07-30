// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using System;
using System.Runtime.CompilerServices;

#include "EngineModelCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_MODELCOLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics.Shapes
{
    public class ModelCollisionShape : CollisionShape, IDestroy
    {
        /// <summary>
        /// The Def used to create the ModelCollisionShape
        /// </summary>
        public ModelCollisionShapeDef ModelDef
        {
            get
            {
                return Def as ModelCollisionShapeDef;
            }
        }

        /// <summary>
        /// Whether the ModelCollisionShape has been Disposed/Finalised
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        ModelCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        internal override void Init()
        {
            ModelCollisionShapeDef def = ModelDef;

            if (def != null)
            {
                string path = ModControl.GetAssetPath(def.ModelPath);
                if (string.IsNullOrEmpty(path))
                {
                    Logger.IcarianError("ModelCollisionShape failed to find model path: " + def.ModelPath);

                    return;
                }

                InternalAddr = ModelCollisionShapeInterop.CreateModel(path);
            }
            else
            {
                Logger.IcarianError($"ModelCollisionShape null Def");
            }
        }
        
        /// <summary>
        /// Disposes of the ModelCollisionShape
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the ModelCollisionShape is being Disposed/Finalised
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
                    Logger.IcarianWarning("ModelCollisionShape failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple ModelCollisionShape Dispose");
            }
        }
        ~ModelCollisionShape()
        {
            Dispose(false);
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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