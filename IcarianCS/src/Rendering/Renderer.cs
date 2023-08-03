using IcarianEngine.Definitions;

namespace IcarianEngine.Rendering
{
    public abstract class Renderer : Component
    {
        public RendererDef RendererDef
        {
            get
            {
                return Def as RendererDef;
            }
        }

        public abstract bool Visible
        {
            get;
            set;
        }

        public abstract Material Material
        {
            get;
            set;
        }
    }
}