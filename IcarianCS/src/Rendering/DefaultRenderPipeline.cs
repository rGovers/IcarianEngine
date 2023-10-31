using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System;
using System.Collections.Generic;

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
        public override Matrix4 PreShadow(Light a_light, Camera a_camera, uint a_textureSlot) 
        {
            // I forget everytime so knicked it
            // https://learnopengl.com/Guest-Articles/2021/CSM
            // On second though something did not look right in the maths so I knicked this
            // https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf

            // I believe I need l near (far / near) ^ (i / N) + (1 - l)(near + (i / N)(far - near))
            List<IRenderTexture> shadowMaps = new List<IRenderTexture>(a_light.ShadowMaps);
            uint cascadeCount = (uint)shadowMaps.Count;
            float near = a_camera.Near;
            float far = a_camera.Far;
            float fnDiff = far - near;
            float fON = far / near;

            // TODO: Tweak this to get better results
            const float Lambda = 0.5f;
            const float InvLambda = 1.0f - Lambda;

            float cN = (float)a_textureSlot / cascadeCount;
            float cF = (float)(a_textureSlot + 1) / cascadeCount;

            float cascadeNear = Lambda * near * Mathf.Pow(fON, cN) + InvLambda * (near + cN * fnDiff);
            if (a_textureSlot == 0)
            {
                cascadeNear = near;
            }
            float cascadeFar = Lambda * near * Mathf.Pow(fON, cF) + InvLambda * (near + cF * fnDiff);
            if (a_textureSlot == cascadeCount - 1)
            {
                cascadeFar = far;
            }

            Matrix4 cameraTrans = a_camera.Transform.ToMatrix();
            Matrix4 proj = a_camera.ToProjection(m_width, m_height, cascadeNear, cascadeFar);
            Matrix4 projInv = Matrix4.Inverse(proj);

            Vector4[] corners = new Vector4[8]
            {
                new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                new Vector4( 1.0f, -1.0f, 1.0f, 1.0f),
                new Vector4( 1.0f,  1.0f, 1.0f, 1.0f),
                new Vector4(-1.0f,  1.0f, 1.0f, 1.0f),
                new Vector4(-1.0f, -1.0f, 0.0f, 1.0f),
                new Vector4( 1.0f, -1.0f, 0.0f, 1.0f),
                new Vector4( 1.0f,  1.0f, 0.0f, 1.0f),
                new Vector4(-1.0f,  1.0f, 0.0f, 1.0f)
            };

            Vector3 mid = Vector3.Zero;
            for (int i = 0; i < 8; ++i)
            {
                corners[i] = corners[i] * projInv;
                corners[i] /= corners[i].W;
                corners[i] = corners[i] * cameraTrans;

                mid += corners[i].XYZ;
            }

            mid /= 8.0f;

            Matrix4 lightTrans = new Matrix4(Vector4.UnitX, Vector4.UnitY, Vector4.UnitZ, new Vector4(mid, 1.0f)) * a_light.Transform.Rotation.ToMatrix();
            Matrix4 lightView = Matrix4.Inverse(lightTrans);

            Vector3 min = Vector3.One * float.MaxValue;
            Vector3 max = Vector3.One * float.MinValue;

            for (int i = 0; i < 8; ++i)
            {
                Vector4 c = lightView * corners[i];

                min.X = Mathf.Min(min.X, c.X);
                min.Y = Mathf.Min(min.Y, c.Y);
                min.Z = Mathf.Min(min.Z, c.Z);

                max.X = Mathf.Max(max.X, c.X);
                max.Y = Mathf.Max(max.Y, c.Y);
                max.Z = Mathf.Max(max.Z, c.Z);
            }

            Vector3 extents = max - min;

            // Ensure stuff behind the light in camera frustum is rendered
            Matrix4 lightProj = Matrix4.CreateOrthographic(extents.X * 2, extents.Y * 2, -extents.Z * 2, extents.Z);

            return lightView * lightProj;
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