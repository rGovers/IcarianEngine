using IcarianEngine.Definitions;
using IcarianEngine.Maths;

namespace IcarianEngine.Rendering.Lighting
{
    public enum LightType : ushort
    {
        Directional = 0,
        Point = 1,
        Spot = 2
    }

    internal enum LightRenderTextureType : ushort
    {
        Null = 0,
        RenderTexture = 1,
        DepthRenderTexture = 2
    }

    internal struct LightRenderTextureBuffer
    {
        public uint RenderTextureAddr;
        public LightRenderTextureType RenderTextureType;
    }

    public abstract class Light : Component
    {
        public abstract LightType LightType
        {
            get;
        }

        public abstract uint RenderLayer
        {
            get;
            set;
        }

        public abstract Color Color
        {
            get;
            set;
        }
        public abstract float Intensity
        {
            get;
            set;
        }

        public LightDef LightDef
        {
            get
            {
                return Def as LightDef;
            }
        }
    }
}