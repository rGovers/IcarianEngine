using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics.Shapes
{
    public class BoxCollisionShape : CollisionShape, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateBox(Vector3 a_extents);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector3 GetExtents(uint a_addr);

        public BoxCollisionShapeDef BoxDef
        {
            get
            {
                return Def as BoxCollisionShapeDef;
            }
        }

        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        public Vector3 Extents
        {
            get
            {
                return GetExtents(InternalAddr);
            }
        }

        public BoxCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        public override void Init()
        {
            BoxCollisionShapeDef def = BoxDef;

            if (def != null)
            {
                InternalAddr = CreateBox(def.Extents);
            }
            else
            {
                InternalAddr = CreateBox(Vector3.One);
            }
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(InternalAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    CollisionShape.DestroyShape(InternalAddr);

                    InternalAddr = uint.MaxValue;
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