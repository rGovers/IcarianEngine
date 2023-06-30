using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System;

namespace IcarianEngine.Rendering
{
    public class DefaultRenderPipeline : RenderPipeline, IDisposable
    {
        MultiRenderTexture m_drawRenderTexture;
        RenderTexture      m_lightRenderTexture;

        TextureSampler     m_colorSampler;
        TextureSampler     m_normalSampler;
        TextureSampler     m_specularSampler;
        TextureSampler     m_emissionSampler;
        TextureSampler     m_depthSampler;

        TextureSampler     m_lightColorSampler;

        uint               m_width;
        uint               m_height;

        void SetTextures(Material a_mat)
        {
            a_mat.SetTexture(0, m_colorSampler);
            a_mat.SetTexture(1, m_normalSampler);
            a_mat.SetTexture(2, m_specularSampler);
            a_mat.SetTexture(3, m_emissionSampler);
            a_mat.SetTexture(4, m_depthSampler);
        }

        void SetPostTextures()
        {
            Material postMat = Material.PostMaterial;

            postMat.SetTexture(0, m_lightColorSampler);
            postMat.SetTexture(1, m_normalSampler);
            postMat.SetTexture(2, m_emissionSampler);
            postMat.SetTexture(3, m_depthSampler);
        }

        public DefaultRenderPipeline()
        {
            m_width = 1920;
            m_height = 1080;

            m_drawRenderTexture = new MultiRenderTexture(4, m_width, m_height, true, true);
            m_lightRenderTexture = new RenderTexture(m_width, m_height, false, true);

            m_colorSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 0);
            m_normalSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 1);
            m_specularSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 2);
            m_emissionSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 3);
            m_depthSampler = TextureSampler.GenerateRenderTextureDepthSampler(m_drawRenderTexture);

            m_lightColorSampler = TextureSampler.GenerateRenderTextureSampler(m_lightRenderTexture);

            SetTextures(Material.DirectionalLightMaterial);
            SetTextures(Material.PointLightMaterial);
            SetTextures(Material.SpotLightMaterial);

            SetPostTextures();
        }

        public override void Resize(uint a_width, uint a_height)
        {
            m_width = a_width;
            m_height = a_height;

            m_drawRenderTexture.Resize(m_width, m_height);
            m_lightRenderTexture.Resize(m_width, m_height);

            SetTextures(Material.DirectionalLightMaterial);
            SetTextures(Material.PointLightMaterial);
            SetTextures(Material.SpotLightMaterial);

            SetPostTextures();
        }

        public override void ShadowSetup(Camera a_camera)
        {
            
        }
        // Multidraw is a bit of a pain so I'm going to leave it for now and do a draw call per shadow map
        // If I can think of a way to do it while still keeping programability I'll do it
        public override void PreShadow(Light a_light, Camera a_camera, uint a_textureSlot) 
        {
            Matrix4 cameraTrans = a_camera.Transform.ToMatrix();
            Matrix4 proj = a_camera.ToProjection(m_width, m_height);
            Matrix4 projInv = Matrix4.Inverse(proj);

            Matrix4 viewProjInv = projInv * cameraTrans;

            // I forget everytime so knicked it
            // https://learnopengl.com/Guest-Articles/2021/CSM
            Vector4[] corners = new Vector4[8];

            for (uint x = 0; x < 2; ++x)
            {
                uint xOff = x * 4;

                for (uint y = 0; y < 2; ++y)
                {
                    uint yOff = y * 2;

                    for (uint z = 0; z < 2; ++z)
                    {
                        Vector4 point = new Vector4
                        (
                            2.0f * x - 1.0f,
                            2.0f * y - 1.0f,
                            2.0f * z - 1.0f,
                            1.0f
                        );

                        Vector4 pointWorld = viewProjInv * point;

                        corners[z + yOff + xOff] = pointWorld / pointWorld.W;
                    }
                }
            }

            Vector3 mid = Vector3.Zero;
            foreach (Vector4 corner in corners)
            {
                mid += corner.XYZ;
            }
            mid /= 8.0f;

            // TODO: Global space instead of local space
            Matrix4 sView = Matrix4.LookAt(mid, a_light.Transform.Forward, a_light.Transform.Up);
        }
        public override void PostShadow(Light a_light, Camera a_camera, uint a_textureSlot)
        {
            
        }

        public override void PreRender(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_drawRenderTexture);
        }
        public override void PostRender(Camera a_camera)
        {
            
        }

        public override void LightSetup(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_lightRenderTexture);
        }

        public override Material PreLight(LightType a_lightType, Camera a_camera)
        {
            Material mat = null;

            switch (a_lightType)
            {
            case LightType.Directional:
            {
                mat = Material.DirectionalLightMaterial;
                
                break;
            }
            case LightType.Point:
            {
                mat = Material.PointLightMaterial;

                break;
            }
            case LightType.Spot:
            {
                mat = Material.SpotLightMaterial;

                break;
            }
            }   
            
            return mat;
        }
        public override void PostLight(LightType a_lightType, Camera a_camera)
        {
            
        }

        public override void PostProcess(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(a_camera.RenderTexture);
            RenderCommand.BindMaterial(Material.PostMaterial);

            RenderCommand.DrawMaterial();
        }

        public virtual void Dispose()
        {
            m_drawRenderTexture.Dispose();
            m_lightRenderTexture.Dispose();

            m_colorSampler.Dispose();
            m_normalSampler.Dispose();
            m_specularSampler.Dispose();
            m_emissionSampler.Dispose();
            m_depthSampler.Dispose();

            m_lightColorSampler.Dispose();
        }
    }
}