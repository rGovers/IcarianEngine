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