using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

#include "EngineCapsuleCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_CAPSULECOLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics.Shapes
{
    public class CapsuleCollisionShape : CollisionShape, IDestroy
    {
        /// <summary>
        /// Definition used to create the CapsuleCollisionShape
        /// </summary>
        public CapsuleCollisionShapeDef CapsuleDef
        {
            get
            {
                return Def as CapsuleCollisionShapeDef;
            }
        }

        /// <summary>
        /// The height of the CapsuleCollisionShape
        /// <summary>
        public float Height
        {
            get
            {
                return CapsuleCollisionShapeInterop.GetHeight(InternalAddr);
            }
        }

        /// <summary>
        /// The radius of the CapsuleCollisionShape
        /// </summary>
        public float Radius
        {
            get
            {
                return CapsuleCollisionShapeInterop.GetRadius(InternalAddr);
            }
        }

        /// <summary>
        /// Whether the CapsuleCollisionShape is Disposed
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        CapsuleCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }
        /// <summary>
        /// Creates a instance of <see cref="IcarianEngine.Physics.Shapes.CapsuleCollisionShape"/>
        /// </summary>
        /// <param name="a_height">The height of the CapsuleCollisionShape</param>
        /// <param name="a_radius">The radius of the CapsuleCollisionShape</param>
        public CapsuleCollisionShape(float a_height, float a_radius)
        {
            InternalAddr = CapsuleCollisionShapeInterop.CreateCapsule(a_height, a_radius);
        }

        internal override void Init()
        {
            CapsuleCollisionShapeDef def = CapsuleDef;

            if (def != null)
            {
                InternalAddr = CapsuleCollisionShapeInterop.CreateCapsule(def.Height, def.Radius);
            }
            else
            {
                InternalAddr = CapsuleCollisionShapeInterop.CreateCapsule(1.0f, 0.5f);
            }
        }

        /// <summary>
        /// Disposes of the CapsuleCollisionShape
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the CapsuleCollisionShape is being Disposed
        /// </summary>
        /// <param name="a_disposing">Whether the CapsuleCollisionShape is Disposed of Finialized</param>
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
                    Logger.IcarianWarning("CapsuleCollisionShape Failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple CapsuleCollisionShape Dispose");
            }
        }
        ~CapsuleCollisionShape()
        {
            Dispose(false);
        }
    };
}