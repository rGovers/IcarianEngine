using IcarianEngine.Definitions;
using System;
using System.Reflection;
using System.Runtime.CompilerServices;

#include "EngineCollisionShapeInterop.h"
#include "InteropBinding.h"

ENGINE_COLLISIONSHAPE_EXPORT_TABLE(IOP_BIND_FUNCTION)

namespace IcarianEngine.Physics.Shapes
{
    public abstract class CollisionShape
    {
        CollisionShapeDef m_def;

        internal uint InternalAddr
        {
            get;
            set;
        }

        /// <summary>
        /// The def used to create the CollisionShape
        /// </summary>
        public CollisionShapeDef Def
        {
            get
            {
                return m_def;
            }
        }

        internal abstract void Init();

        /// <summary>
        /// Creates a CollisionShape from a def of type T
        /// <summary>
        /// <param name="a_def">The def to use to create the CollisionShape</param>
        /// <returns>The CollisionShape of type T. Null on failure.</returns>
        public static T FromDef<T>(CollisionShapeDef a_def) where T : CollisionShape
        {
            return FromDef(a_def) as T;
        }

        /// <summary>
        /// Creates a CollisionShape from a def
        /// <summary>
        /// <param name="a_def">The def to use to create the CollisionShape</param>
        /// <returns>The CollisionShape. Null on failure</returns>
        public static CollisionShape FromDef(CollisionShapeDef a_def)
        {
            CollisionShape shape = Activator.CreateInstance(a_def.CollisionShapeType, true) as CollisionShape;
            if (shape != null)
            {
                shape.m_def = a_def;
                shape.Init();
            }
            else
            {
                Logger.IcarianWarning($"Could not create CollisionShape from Def: {a_def}");
            }

            return shape;
        }
    }
}