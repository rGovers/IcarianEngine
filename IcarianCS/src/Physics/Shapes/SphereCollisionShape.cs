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