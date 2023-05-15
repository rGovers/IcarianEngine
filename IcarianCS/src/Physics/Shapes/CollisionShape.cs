using System;
using IcarianEngine.Definitions;

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

        public CollisionShapeDef Def
        {
            get
            {
                return m_def;
            }
        }

        public abstract void Init();

        public static T FromDef<T>(CollisionShapeDef a_def) where T : CollisionShape
        {
            return FromDef(a_def) as T;
        }

        public static CollisionShape FromDef(CollisionShapeDef a_def)
        {
            CollisionShape shape = Activator.CreateInstance(a_def.CollisionShapeType) as CollisionShape;
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