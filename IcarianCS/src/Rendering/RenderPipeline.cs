// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public struct LightShadowPass
    {
        /// <summary>
        /// The split infomation for the shadow pass
        /// </summary>
        public LightShadowSplit[] Splits;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Material" /> to use for the shadow light
        /// </summary>
        public Material Material;
    };

    /// <summary>
    /// Render Pipeline used to control render passes
    /// </summary>
    /// Functions are called asynchonously from render pass thread(s) in <see cref="IcarianEngine.ThreadPool" />.
    /// Refer to <see cref="IcarianEngine.Rendering.DefaultRenderPipeline" /> for the default implementation
    /// @warning Thread safety is not guaranteed
    /// @warning Order of calls is not guaranteed
    public abstract class RenderPipeline
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetLightSplits(LightShadowSplit[] a_splits);

        static RenderPipeline s_instance = null;

        /// <summary>
        /// The current RenderPipeline
        /// </summary>
        public static RenderPipeline Instance
        {
            get
            {
                return s_instance;
            }
        }

        /// <summary>
        /// Called when the swap chain is resized
        /// </summary>
        /// <param name="a_width">The new width of the swapchain</param>
        /// <param name="a_height">The new height of the swapchain</param>
        public abstract void Resize(uint a_width, uint a_height);

        /// <summary>
        /// Called before the shadow pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_lightType">The type of <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow pass is for</param>
        public abstract void ShadowSetup(LightType a_lightType, Camera a_camera);
        /// <summary>
        /// Called before each split of the shadow pass for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow pass is for</param>
        /// <param name="a_textureSlot">The texture slot to render the shadow map to</param>
        /// <returns>Infomation to use for the shadow pass</returns>
        public abstract LightShadowSplit PreShadow(ShadowLight a_light, Camera a_camera, uint a_textureSlot);
        /// <summary>
        /// Called after each split of the shadow pass for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow pass is for</param>
        /// <param name="a_textureSlot">The texture slot to render the shadow map to</param>
        public abstract void PostShadow(ShadowLight a_light, Camera a_camera, uint a_textureSlot);

        /// <summary>
        /// Called before the main render pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the render pass is for</param>
        public abstract void PreRender(Camera a_camera);
        /// <summary>
        /// Called after the main render pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the render pass is for</param>
        public abstract void PostRender(Camera a_camera);

        /// <summary>
        /// Called before the light pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light pass is for</param>
        public abstract void LightSetup(Camera a_camera);
        /// <summary>
        /// Called before shadow casting lights before the light pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light pass is for</param>
        /// <returns>Infomation to use for the light shadow pass</returns>
        public abstract LightShadowPass PreShadowLight(ShadowLight a_light, Camera a_camera);
        /// <summary>
        /// Called after shadow casting lights before the light pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light pass is for</param>
        public abstract void PostShadowLight(ShadowLight a_light, Camera a_camera);
        /// <summary>
        /// Called before the light pass for a light type for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_lightType">The type of <see cref="IcarianEngine.Rendering.Lighting.Light" /> the pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light pass is for</param>
        /// <returns>The material to use for the light pass</returns>
        public abstract Material PreLight(LightType a_lightType, Camera a_camera);
        /// <summary>
        /// Called after the light pass for a light type for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_lightType">The type of <see cref="IcarianEngine.Rendering.Lighting.Light" /> the pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the <see cref="IcarianEngine.Rendering.Lighting.Light" /> pass is for</param>
        public abstract void PostLight(LightType a_lightType, Camera a_camera);

        /// <summary>
        /// Called before the forward pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the forward pass is for</param>
        public abstract void PreForward(Camera a_camera);
        /// <summary>
        /// Called after the forward pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the forward pass if for</param>
        public abstract void PostForward(Camera a_camera);

        /// <summary>
        /// Called for the post processing pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the post process pass is for</param>
        public abstract void PostProcess(Camera a_camera);

        /// <summary>
        /// Sets the current render pipeline
        /// </summary>
        /// <param name="a_pipeline">The RenderPipeline to use</param>
        /// Disposes of the old RenderPipeline if it is Disposable
        public static void SetPipeline(RenderPipeline a_pipeline)
        {
            RenderPipeline oldPipeline = s_instance;

            s_instance = a_pipeline;
            
            if (oldPipeline is IDisposable disp)
            {
                disp.Dispose();
            }
        }
        internal static void Destroy()
        {
            if (s_instance is IDisposable disp)
            {
                disp.Dispose();
                s_instance = null;
            }
        }
        
        static void ShadowSetupS(uint a_lightType, uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);
                LightType type = (LightType)a_lightType;

                if (cam != null)
                {
                    s_instance.ShadowSetup(type, cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PreShadowS(uint a_lightType, uint a_lightIndex, uint a_camBuffer, uint a_textureSlot)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);
                ShadowLight light = null;
                LightType type = (LightType)a_lightType;
                switch (type)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

                    break;
                }
                case LightType.Spot:
                {
                    light = SpotLight.GetLight(a_lightIndex);

                    break;
                }
                }

                if (cam != null && light != null)
                {
                    LightShadowSplit split = s_instance.PreShadow(light, cam, a_textureSlot);

                    SetLightSplits(new LightShadowSplit[] { split });
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostShadowS(uint a_lightType, uint a_lightIndex, uint a_camBuffer, uint a_textureSlot)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);
                ShadowLight light = null;
                switch ((LightType)a_lightType)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

                    break;
                }
                case LightType.Spot:
                {
                    light = SpotLight.GetLight(a_lightIndex);

                    break;
                }
                }

                if (cam != null && light != null)
                {
                    s_instance.PostShadow(light, cam, a_textureSlot);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            } 
        }

        static void PreRenderS(uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.PreRender(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostRenderS(uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.PostRender(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }

        static void LightSetupS(uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.LightSetup(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PreShadowLightS(uint a_lightType, uint a_lightIndex, uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);
                ShadowLight light = null;
                LightType type = (LightType)a_lightType;

                switch (type)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

                    break;
                }
                case LightType.Point:
                {
                    light = PointLight.GetLight(a_lightIndex);

                    break;
                }
                case LightType.Spot:
                {
                    light = SpotLight.GetLight(a_lightIndex);

                    break;
                }
                }

                if (cam != null && light != null)
                {
                    LightShadowPass pass = s_instance.PreShadowLight(light, cam);

                    RenderCommand.BindMaterial(pass.Material);

                    if (pass.Splits != null)
                    {
                        SetLightSplits(pass.Splits);
                    }
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostShadowLightS(uint a_lightType, uint a_lightIndex, uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);
                ShadowLight light = null;

                switch ((LightType)a_lightType)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

                    break;
                }
                case LightType.Point:
                {
                    light = PointLight.GetLight(a_lightIndex);

                    break;
                }
                case LightType.Spot:
                {
                    light = SpotLight.GetLight(a_lightIndex);

                    break;
                }
                }

                if (cam != null && light != null)
                {
                    s_instance.PostShadowLight(light, cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PreLightS(uint a_lightType, uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Material mat = s_instance.PreLight((LightType)a_lightType, cam);

                    RenderCommand.BindMaterial(mat);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostLightS(uint a_lightType, uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.PostLight((LightType)a_lightType, cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipelien not initialized");
            }
        }

        static void PreForwardS(uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.PreForward(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostForwardS(uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.PostForward(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }

        static void PostProcessS(uint a_camBuffer)
        {
            if (s_instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    s_instance.PostProcess(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }

        static void ResizeS(uint a_width, uint a_height)
        {
            if (s_instance != null)
            {
                s_instance.Resize(a_width, a_height);
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
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