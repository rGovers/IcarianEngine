using System;
using System.Runtime.CompilerServices;
using IcarianEngine.Definitions;

namespace IcarianEngine.Physics.Shapes
{
    public class CylinderCollisionShape : CollisionShape, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateCylinder(float a_height, float a_radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetHeight(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetRadius(uint a_addr);

        public CylinderCollisionShapeDef CylinderDef
        {
            get
            {
                return Def as CylinderCollisionShapeDef;
            }
        }

        public float Height
        {
            get
            {
                return GetHeight(InternalAddr);
            }
        }
        public float Radius
        {
            get
            {
                return GetRadius(InternalAddr);
            }
        }

        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        public CylinderCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        public override void Init()
        {
            CylinderCollisionShapeDef def = CylinderDef;

            if (def != null)
            {
                InternalAddr = CreateCylinder(def.Height, def.Radius);
            }
            else
            {
                InternalAddr = CreateCylinder(1.0f, 0.5f);
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