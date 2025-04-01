// Icarian Engine - C# Game Engine
// 
// License at end of file.

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
        /// Called when the Swapchain is resized
        /// </summary>
        /// <param name="a_width">The new width of the swapchain</param>
        /// <param name="a_height">The new height of the swapchain</param>
        public virtual void Resize(uint a_width, uint a_height) { } 

        /// <summary>
        /// Called when the PostEffect needs to be run
        /// </summary>
        /// <param name="a_renderTexture">The target <see cref="IcarianEngine.Rendering.IRenderTexture" /></param>
        /// <param name="a_samplers">Samplers used by the RenderPipeline</param>
        /// <param name="a_gBuffer">The Deffered <see cref="IcarianEngine.Rendering.MultiRenderTexture" /> used for rendering</param>
        public abstract void Run(IRenderTexture a_renderTexture, TextureSampler[] a_samplers, MultiRenderTexture a_gBuffer);
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.