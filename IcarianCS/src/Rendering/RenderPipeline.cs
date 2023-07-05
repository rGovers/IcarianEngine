using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    public abstract class RenderPipeline
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetLightLVP(float[] a_lvp);

        static RenderPipeline Instance = null;

        public abstract void ShadowSetup(Camera a_camera);
        public abstract Matrix4 PreShadow(Light a_light, Camera a_camera, uint a_textureSlot);
        public abstract void PostShadow(Light a_light, Camera a_camera, uint a_textureSlot);

        public abstract void PreRender(Camera a_camera);
        public abstract void PostRender(Camera a_camera);

        public abstract void LightSetup(Camera a_camera);
        public abstract Material PreLight(LightType a_lightType, Camera a_camera);
        public abstract void PostLight(LightType a_lightType, Camera a_camera);

        public abstract void Resize(uint a_width, uint a_height);
        public abstract void PostProcess(Camera a_camera);

        public static void Init(RenderPipeline a_pipeline)
        {
            if (Instance is IDisposable disp)
            {
                disp.Dispose();
            }

            Instance = a_pipeline;
        }
        internal static void Destroy()
        {
            if (Instance is IDisposable disp)
            {
                disp.Dispose();
                Instance = null;
            }
        }
        
        static void ShadowSetupS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.ShadowSetup(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PreShadowS(uint a_lightType, uint a_lightIndex, uint a_camBuffer, uint a_textureSlot)
        {
            if (Instance != null)
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
                    Matrix4 mat = Instance.PreShadow(light, cam, a_textureSlot);

                    SetLightLVP(mat.ToArray());
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostShadowS(uint a_lightType, uint a_lightIndex, uint a_camBuffer, uint a_textureSlot)
        {
            if (Instance != null)
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
                    Instance.PostShadow(light, cam, a_textureSlot);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            } 
        }

        static void PreRenderS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PreRender(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PostRenderS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostRender(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }

        static void LightSetupS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.LightSetup(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
        static void PreLightS(uint a_lightType, uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Material mat = Instance.PreLight((LightType)a_lightType, cam);

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
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostLight((LightType)a_lightType, cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipelien not initialized");
            }
        }

        static void PostProcessS(uint a_camBuffer)
        {
            if (Instance != null)
            {
                Camera cam = Camera.GetCamera(a_camBuffer);

                if (cam != null)
                {
                    Instance.PostProcess(cam);
                }
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }

        static void ResizeS(uint a_width, uint a_height)
        {
            if (Instance != null)
            {
                Instance.Resize(a_width, a_height);
            }
            else
            {
                Logger.IcarianError("RenderPipeline not initialized");
            }
        }
    }
}