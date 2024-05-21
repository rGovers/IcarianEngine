namespace IcarianEngine.Rendering.PostEffects
{
    public abstract class PostEffect
    {
        public virtual bool ShouldRun
        {
            get
            {
                return true;
            }
        }

        public virtual void Resize(uint a_width, uint a_height) { } 

        public abstract void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers);
    }
}