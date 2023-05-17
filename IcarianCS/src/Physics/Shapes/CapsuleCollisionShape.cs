using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics.Shapes
{
    public class CapsuleCollisionShape : CollisionShape, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreateCapsule(float a_height, float a_radius);  
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetHeight(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float GetRadius(uint a_addr);

        public CapsuleCollisionShapeDef CapsuleDef
        {
            get
            {
                return Def as CapsuleCollisionShapeDef;
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

        public CapsuleCollisionShape()
        {
            InternalAddr = uint.MaxValue;
        }

        public override void Init()
        {
            CapsuleCollisionShapeDef def = CapsuleDef;

            if (def != null)
            {
                InternalAddr = CreateCapsule(def.Height, def.Radius);
            }
            else
            {
                InternalAddr = CreateCapsule(1.0f, 0.5f);
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