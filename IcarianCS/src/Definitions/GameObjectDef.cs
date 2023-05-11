using IcarianEngine.Maths;
using System;
using System.Collections.Generic;

namespace IcarianEngine.Definitions
{
    public class GameObjectDef : Def
    {
        public Type ObjectType = typeof(GameObject);

        [EditorTooltip("GameObject position offset.")]
        public Vector3 Translation = Vector3.Zero;
        [EditorTooltip("GameObject rotation offset.")]
        public Quaternion Rotation = Quaternion.Identity;
        [EditorTooltip("GameObject scale offset.")]
        public Vector3 Scale = Vector3.One;

        [EditorTooltip("List of components the GameObject is composed of.")]
        public List<ComponentDef> Components = new List<ComponentDef>();

        [EditorTooltip("GameObject children.")]
        public List<GameObjectDef> Children = new List<GameObjectDef>();

        public override void PostResolve()
        {
            base.PostResolve();

            if (ObjectType != typeof(GameObject) && !ObjectType.IsSubclassOf(typeof(GameObject)))
            {
                Logger.IcarianError($"Game Object Def Invalid ObjectType: {ObjectType}");

                return;
            }
        }
    }
}