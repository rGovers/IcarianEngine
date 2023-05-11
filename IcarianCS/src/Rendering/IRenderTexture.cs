namespace IcarianEngine.Rendering
{
    public interface IRenderTexture : IDestroy
    {
        uint Width
        {
            get;
        }
        uint Height
        {
            get;
        }

        bool HasDepth
        {
            get;
        }

        void Resize(uint a_width, uint a_height);
    }
}