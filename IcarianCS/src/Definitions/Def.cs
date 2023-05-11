using System;

namespace IcarianEngine.Definitions
{
    public class Def
    {
        [HideInEditor, NonSerialized]
        public string DefName = string.Empty;
        [HideInEditor, NonSerialized]
        public string DefPath = string.Empty;
        [HideInEditor, NonSerialized]
        public string DefParentName = string.Empty;

        public const string SceneDefPath = "[Scene]";

        public bool IsSceneDef
        {
            get
            {
                return DefPath == SceneDefPath;
            }
        }

        public virtual void PostResolve() { }
    };
}