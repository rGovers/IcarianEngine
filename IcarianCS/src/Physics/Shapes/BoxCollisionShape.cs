using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;

#include "EngineBoxCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_BOXCOLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics.Shapes
{
    public class BoxCollisionShape : CollisionShape, IDestroy
    {
        /// <summary>
        /// The Definition used to create the BoxCollisionShape
        /// </summary>
        public BoxCollisionShapeDef BoxDef
        {
            get
            {
                return Def as BoxCollisionShapeDef;
            }
        }

        /// <summary>
        /// Determines if the BoxCollisionShape is diposed
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        /// <summary>
        /// The extents of the box
        /// </summary>
        public Vector3 Extents
        {
            get
            {
                return BoxCollisionShapeInterop.GetExtents(InternalAddr);
            }
        }

        BoxCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        /// <summary>
        /// Creates a instance of <see cref="IcarianEngine.Physics.Shapes.BoxCollisionShape"/>
        /// </summary>
        /// <param name="a_extents">The extents of the BoxCollisionShape</param>
        public BoxCollisionShape(Vector3 a_extents)
        {
            InternalAddr = BoxCollisionShapeInterop.CreateBox(a_extents);
        }

        internal override void Init()
        {
            BoxCollisionShapeDef def = BoxDef;

            if (def != null)
            {
                InternalAddr = BoxCollisionShapeInterop.CreateBox(def.Extents);
            }
            else
            {
                InternalAddr = BoxCollisionShapeInterop.CreateBox(Vector3.One);
            }
        }

        /// <summary> 
        /// Disposes of the BoxCollisionShape
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the BoxCollisionShape is being Disposed
        /// </summary>
        /// <param name="a_disposing">Whether the BoxCollisionShape is Disposed or Finalized</param>
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
                    Logger.IcarianWarning("BoxCollisionShape Failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple BoxCollisionShape Dispose");
            }
        }
        ~BoxCollisionShape()
        {
            Dispose(false);
        }
    }
}