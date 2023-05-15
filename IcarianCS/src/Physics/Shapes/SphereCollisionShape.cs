using System;
using System.Runtime.CompilerServices;
using IcarianEngine.Definitions;

namespace IcarianEngine.Physics.Shapes
{
    public class SphereCollisionShape : CollisionShape, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateSphere(float a_radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetRadius(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroySphere(uint a_addr);

        public SphereCollisionShapeDef SphereDef
        {
            get
            {
                return Def as SphereCollisionShapeDef;
            }
        }

        public bool IsDisposed
        {
            get
            {
                return InternalAddr == uint.MaxValue;
            }
        }

        public float Radius
        {
            get
            {
                return GetRadius(InternalAddr);
            }
        }

        public SphereCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        public override void Init()
        {
            SphereCollisionShapeDef def = SphereDef;

            if (def != null)
            {
                InternalAddr = CreateSphere(def.Radius);
            }
            else
            {
                InternalAddr = CreateSphere(1.0f);
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
                    DestroySphere(InternalAddr);

                    InternalAddr = uint.MaxValue;
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