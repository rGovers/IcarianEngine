using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public struct LightShadowSplit
    {
        /// <summary>
        /// The far plane of the split
        /// </summary>
        public float Split;
        /// <summary>
        /// The light view projection matrix for the split
        /// </summary>
        public Matrix4 LVP;
    }

    public struct LightShadowPass
    {
        /// <summary>
        /// The split infomation for the shadow pass
        /// </summary>
        public LightShadowSplit[] Splits;
        /// <summary>
        /// The material to use for the shadow light
        /// </summary>
        public Material Material;
    };

    /// <summary>
    /// Render Pipeline used to control render passes
    /// </summary>
    /// Functions are called asynchonously from render pass thread(s) in <see cref="IcarianEngine.ThreadPool"/>.
    /// Thread safety is not guaranteed.
    /// Order of calls is not guaranteed.
    public abstract class RenderPipeline
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetLightLVP(float[][] a_lvp);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetLightSplits(float[] a_splits);

        static RenderPipeline s_instance = null;

        /// <summary>
        /// The current render pipeline
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
        public abstract void Resize(uint a_width, uint a_height);

        /// <summary>
        /// Called before the shadow pass
        /// </summary>
        /// <param name="a_lightType">The type of light the shadow pass is for</param>
        /// <param name="a_camera">The camera the shadow pass is for</param>
        public abstract void ShadowSetup(LightType a_lightType, Camera a_camera);
        /// <summary>
        /// Called before each split of the shadow pass for a light
        /// </summary>
        /// <param name="a_light">The light the shadow pass is for</param>
        /// <param name="a_camera">The camera the shadow pass is for</param>
        /// <param name="a_textureSlot">The texture slot to render the shadow map to</param>
        /// <returns>Infomation to use for the shadow pass</returns>
        public abstract LightShadowSplit PreShadow(Light a_light, Camera a_camera, uint a_textureSlot);
        /// <summary>
        /// Called after each split of the shadow pass for a light
        /// </summary>
        /// <param name="a_light">The light the shadow pass is for</param>
        /// <param name="a_camera">The camera the shadow pass is for</param>
        /// <param name="a_textureSlot">The texture slot to render the shadow map to</param>
        public abstract void PostShadow(Light a_light, Camera a_camera, uint a_textureSlot);

        /// <summary>
        /// Called before the main render pass
        /// </summary>
        /// <param name="a_camera">The camera the render pass is for</param>
        public abstract void PreRender(Camera a_camera);
        /// <summary>
        /// Called after the main render pass
        /// </summary>
        /// <param name="a_camera">The camera the render pass is for</param>
        public abstract void PostRender(Camera a_camera);

        /// <summary>
        /// Called before the light pass
        /// </summary>
        /// <param name="a_camera">The camera the light pass is for</param>
        public abstract void LightSetup(Camera a_camera);
        /// <summary>
        /// Called before shadow casting lights before the light pass
        /// </summary>
        /// <param name="a_light">The shadowed light the pass is for</param>
        /// <param name="a_camera">The camera the light pass is for</param>
        /// <returns>Infomation to use for the light shadow pass</returns>
        public abstract LightShadowPass PreShadowLight(Light a_light, Camera a_camera);
        /// <summary>
        /// Called after shadow casting lights before the light pass
        /// </summary>
        /// <param name="a_light">The shadowed light the pass is for</param>
        /// <param name="a_camera">The camera the light pass is for</param>
        public abstract void PostShadowLight(Light a_light, Camera a_camera);
        /// <summary>
        /// Called before the light pass for a light type
        /// </summary>
        /// <param name="a_lightType">The type of light the pass is for</param>
        /// <param name="a_camera">The camera the light pass is for</param>
        /// <returns>The material to use for the light pass</returns>
        public abstract Material PreLight(LightType a_lightType, Camera a_camera);
        /// <summary>
        /// Called after the light pass for a light type
        /// </summary>
        /// <param name="a_lightType">The type of light the pass is for</param>
        /// <param name="a_camera">The camera the light pass is for</param>
        public abstract void PostLight(LightType a_lightType, Camera a_camera);

        /// <summary>
        /// Called for the post processing pass
        /// </summary>
        /// <param name="a_camera">The camera the post process pass is for</param>
        public abstract void PostProcess(Camera a_camera);

        /// <summary>
        /// Sets the current render pipeline
        /// </summary>
        /// <param name="a_pipeline">The render pipeline to use</param>
        /// Disposes of the old render pipeline if it is disposable
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
                Light light = null;
                LightType type = (LightType)a_lightType;
                switch (type)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

                    break;
                }
                }

                if (cam != null && light != null)
                {
                    LightShadowSplit split = s_instance.PreShadow(light, cam, a_textureSlot);

                    SetLightLVP(new float[][] { split.LVP.ToArray() });

                    if (type == LightType.Directional)
                    {
                        SetLightSplits(new float[] { split.Split });
                    }
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
                Light light = null;
                switch ((LightType)a_lightType)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

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
                Light light = null;
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
                }

                if (cam != null && light != null)
                {
                    LightShadowPass pass = s_instance.PreShadowLight(light, cam);

                    RenderCommand.BindMaterial(pass.Material);

                    if (pass.Splits != null)
                    {
                        int count = pass.Splits.Length;

                        float[][] lvp = new float[count][];
                        for (int i = 0; i < count; ++i)
                        {
                            lvp[i] = pass.Splits[i].LVP.ToArray();
                        }

                        SetLightLVP(lvp);

                        if (type == LightType.Directional)
                        {
                            float[] splits = new float[count];
                            for (int i = 0; i < count; ++i)
                            {
                                splits[i] = pass.Splits[i].Split;
                            }

                            SetLightSplits(splits);
                        }
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
                Light light = null;

                switch ((LightType)a_lightType)
                {
                case LightType.Directional:
                {
                    light = DirectionalLight.GetLight(a_lightIndex);

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