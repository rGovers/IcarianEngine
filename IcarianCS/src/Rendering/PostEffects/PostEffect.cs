namespace IcarianEngine.Rendering.PostEffects
{
    public abstract class PostEffect
    {
        /// <summary>
        /// Should run the PostEffect
        /// </summary>
        public virtual bool ShouldRun
        {
            get
            {
                return true;
            }
        }

        /// <summary>
        /// Called when the swapchain is resized
        /// </summary>
        /// <param name="a_width">The new width of the swapchain</param>
        /// <param name="a_height">The new height of the swapchain</param>
        public virtual void Resize(uint a_width, uint a_height) { } 

        /// <summary>
        /// Called when the post effect need to be run
        /// </summary>
        /// <param name="a_renderTexture">The target <see cref="IcarianEngine.Rendering.IRenderTexture" /></param>
        /// <param name="a_samplers">Samplers used by the RenderPipeline</param>
        public abstract void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers);
    }
}