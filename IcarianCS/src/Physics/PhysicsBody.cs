using IcarianEngine.Definitions;
using IcarianEngine.Physics.Shapes;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Physics
{
    public class PhysicsBody : Component, IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CreatePhysicsBody(uint a_transformAddr, uint a_colliderAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyPhysicsBody(uint a_addr);

        public PhysicsBodyDef PhysicsBodyDef
        {
            get
            {
                return Def as PhysicsBodyDef;
            }
        }

        bool           m_disposed = false;

        CollisionShape m_collisionShape = null;

        internal uint InternalAddr
        {
            get;
            set;
        }

        public bool IsDisposed
        {
            get
            {
                return m_disposed;
            }
        }

        public CollisionShape CollisionShape
        {
            get
            {
                return m_collisionShape;
            }
            set
            {
                CollisionShapeSet(m_collisionShape, value);

                m_collisionShape = value;
            }
        }

        public PhysicsBody()
        {
            InternalAddr = uint.MaxValue;
        }        

        public override void Init()
        {
            base.Init();

            PhysicsBodyDef def = PhysicsBodyDef;
            if (def != null)
            {
                CollisionShape = AssetLibrary.GetCollisionShape(def.CollisionShape);
            }
        }

        protected internal virtual void CollisionShapeSet(CollisionShape a_oldShape, CollisionShape a_newShape)
        {
            if (InternalAddr != uint.MaxValue)
            {
                DestroyPhysicsBody(InternalAddr);

                InternalAddr = uint.MaxValue;
            }

            if (a_newShape != null)
            {
                InternalAddr = CreatePhysicsBody(Transform.InternalAddr, a_newShape.InternalAddr);
            }
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(!m_disposed)
            {
                if(a_disposing)
                {
                    CollisionShape = null;

                    m_disposed = true;
                }
                else
                {
                    Logger.IcarianWarning("PhysicsBody Failed to Dispose");
                }

                InternalAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple PhysicsBody Dispose");
            }
        }

        ~PhysicsBody()
        {
            Dispose(false);
        }
    }
}