using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using IcarianEngine.Rendering.PostEffects;
using System;
using System.Collections.Generic;

namespace IcarianEngine.Rendering
{
    public class DefaultRenderPipeline : RenderPipeline, IDisposable
    {
        /// <summary>
        /// The number of cascades to use for <see cref="IcarianEngine.Rendering.Lighting.DirectionalLight" /> shadows
        /// </summary>
        public const uint CascadeCount = 6;
        public const uint PostTextureStackSize = 2;

        VertexShader       m_quadVert;
        PixelShader        m_ambientLightPixel;
        PixelShader        m_directionalLightPixel;
        PixelShader        m_pointLightPixel;
        PixelShader        m_spotLightPixel;
        PixelShader        m_directionalLightShadowPixel;
        PixelShader        m_pointLightShadowPixel;
        PixelShader        m_spotLightShadowPixel;
        PixelShader        m_blendPixel;

        Material           m_ambientLightMaterial;
        Material           m_directionalLightMaterial;
        Material           m_pointLightMaterial;
        Material           m_spotLightMaterial;
        Material           m_directionalLightShadowMaterial;
        Material           m_pointLightShadowMaterial;
        Material           m_spotLightShadowMaterial;
        Material           m_blendMaterial;

        DepthRenderTexture m_depthRenderTexture;

        MultiRenderTexture m_drawRenderTexture;

        RenderTexture      m_lightRenderTexture;
        RenderTexture      m_forwardRenderTexture;
        RenderTexture      m_colorRenderTexture;

        TextureSampler[]   m_postTextureSamplers;
        RenderTexture[]    m_postRenderTextures;

        List<PostEffect>   m_postEffects;

        TextureSampler     m_defferedColorSampler;
        TextureSampler     m_normalSampler;
        TextureSampler     m_specularSampler;
        TextureSampler     m_emissionSampler;
        TextureSampler     m_depthSampler;

        TextureSampler     m_lightColorSampler;
        TextureSampler     m_forwardSampler;

        TextureSampler     m_colorSampler;

        uint               m_width;
        uint               m_height;

        float              m_renderScale;
        float              m_shadowCutoff;
        float              m_lambda;

        /// <summary>
        /// The lambda value for cascaded shadow maps
        /// </summary>
        public float CascadeLambda
        {
            get
            {
                return m_lambda;
            }
            set
            {
                m_lambda = value;
            }
        }

        /// <summary>
        /// The render scale of the Pipeline
        /// </summary>
        public float RenderScale
        {
            get
            {
                return m_renderScale;
            }
        }

        /// <summary>
        /// The shadow cutoff for <see cref="IcarianEngine.Rendering.Lighting.DirectionalLight" />
        /// </summary>
        /// 0-1 range of the far plane to use for shadows
        public float ShadowCutoff
        {
            get
            {
                return m_shadowCutoff;
            }
            set
            {
                m_shadowCutoff = value;
            }
        }

        /// <summary>
        /// Gets a list of <see cref="IcarianEngine.Rendering.PostEffect" /> for the RenderPipeline
        /// </summary>
        public IEnumerable<PostEffect> PostEffects
        {
            get
            {
                return m_postEffects;
            }
        }

        void SetLightTextures(Material a_mat)
        {
            a_mat.SetTexture(0, m_defferedColorSampler);
            a_mat.SetTexture(1, m_normalSampler);
            a_mat.SetTexture(2, m_specularSampler);
            a_mat.SetTexture(3, m_emissionSampler);
            a_mat.SetTexture(4, m_depthSampler);
        }

        void SetBlendTextures()
        {
            m_blendMaterial.SetTexture(0, m_lightColorSampler);
            m_blendMaterial.SetTexture(1, m_forwardSampler);
        }

        public DefaultRenderPipeline(IEnumerable<PostEffect> a_effects)
        {
            m_postEffects = new List<PostEffect>();

            m_lambda = 0.5f;
            m_shadowCutoff = 0.5f;

            m_width = 1280;
            m_height = 720;
            m_renderScale = 1.0f;

            m_depthRenderTexture = new DepthRenderTexture(m_width, m_height);

            m_drawRenderTexture = new MultiRenderTexture(4, m_width, m_height, m_depthRenderTexture, true);

            m_lightRenderTexture = new RenderTexture(m_width, m_height, false, true);
            m_forwardRenderTexture = new RenderTexture(m_width, m_height, m_depthRenderTexture, true);

            m_colorRenderTexture = new RenderTexture(m_width, m_height, false, true);

            m_defferedColorSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 0);
            m_normalSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 1);
            m_specularSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 2);
            m_emissionSampler = TextureSampler.GenerateRenderTextureSampler(m_drawRenderTexture, 3);
            m_depthSampler = TextureSampler.GenerateRenderTextureDepthSampler(m_drawRenderTexture);

            m_lightColorSampler = TextureSampler.GenerateRenderTextureSampler(m_lightRenderTexture);
            m_forwardSampler = TextureSampler.GenerateRenderTextureSampler(m_forwardRenderTexture);

            m_colorSampler = TextureSampler.GenerateRenderTextureSampler(m_colorRenderTexture);

            m_postRenderTextures = new RenderTexture[PostTextureStackSize];
            m_postTextureSamplers = new TextureSampler[PostTextureStackSize];
            for (uint i = 0; i < PostTextureStackSize; ++i)
            {
                m_postRenderTextures[i] = new RenderTexture(m_width, m_height, false, true);
                m_postTextureSamplers[i] = TextureSampler.GenerateRenderTextureSampler(m_postRenderTextures[i]);
            }

            m_quadVert = VertexShader.LoadVertexShader("[INTERNAL]Quad");

            m_ambientLightPixel = PixelShader.LoadPixelShader("[INTERNAL]AmbientLight");
            m_directionalLightPixel = PixelShader.LoadPixelShader("[INTERNAL]DirectionalLight");
            m_pointLightPixel = PixelShader.LoadPixelShader("[INTERNAL]PointLight");
            m_spotLightPixel = PixelShader.LoadPixelShader("[INTERNAL]SpotLight");
            m_directionalLightShadowPixel = PixelShader.LoadPixelShader("[INTERNAL]DirectionalLightShadow");
            m_pointLightShadowPixel = PixelShader.LoadPixelShader("[INTERNAL]PointLightShadow");
            m_spotLightShadowPixel = PixelShader.LoadPixelShader("[INTERNAL]SpotLightShadow");
            m_blendPixel = PixelShader.LoadPixelShader("[INTERNAL]Blend");

            MaterialBuilder ambientLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_ambientLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };

            MaterialBuilder directionalLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_directionalLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };
            
            MaterialBuilder pointLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_pointLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };

            MaterialBuilder spotLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_spotLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };

            m_ambientLightMaterial = Material.CreateMaterial(ambientLightBuilder);
            m_directionalLightMaterial = Material.CreateMaterial(directionalLightBuilder);
            m_pointLightMaterial = Material.CreateMaterial(pointLightBuilder);
            m_spotLightMaterial = Material.CreateMaterial(spotLightBuilder);

            SetLightTextures(m_ambientLightMaterial);
            SetLightTextures(m_directionalLightMaterial);
            SetLightTextures(m_pointLightMaterial);
            SetLightTextures(m_spotLightMaterial);

            MaterialBuilder directionalLightShadowMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_directionalLightShadowPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };

            MaterialBuilder pointLightShadowMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_pointLightShadowPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };

            MaterialBuilder spotLightShadowMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_spotLightShadowPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.One
            };

            m_directionalLightShadowMaterial = Material.CreateMaterial(directionalLightShadowMaterial);
            m_pointLightShadowMaterial = Material.CreateMaterial(pointLightShadowMaterial);
            m_spotLightShadowMaterial = Material.CreateMaterial(spotLightShadowMaterial);

            SetLightTextures(m_directionalLightShadowMaterial);
            SetLightTextures(m_pointLightShadowMaterial);
            SetLightTextures(m_spotLightShadowMaterial);

            MaterialBuilder blendMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_blendPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                ColorBlendMode = MaterialBlendMode.None
            };

            m_blendMaterial = Material.CreateMaterial(blendMaterial);

            SetBlendTextures();

            m_postEffects.AddRange(a_effects);
        }

        /// <summary>
        /// Adds a <see cref="IcarianEngine.Rendering.PostEffect" /> to the post processing stack
        /// </summary>
        /// <param name="a_postEffect"><see cref="IcarianEngine.Rendering.PostEffect" /> to add</param>
        public void AddPostEffect(PostEffect a_postEffect)
        {
            if (m_postEffects.Contains(a_postEffect))
            {
                return;
            }

            m_postEffects.Add(a_postEffect);
        }
        /// <summary>
        /// Removes a <see cref="IcarianEngine.Rendering.PostEffect" /> from the post processing stack
        /// </summary>
        /// <param name="a_postEffect"><see cref="IcarianEngine.Rendering.PostEffect" /> to remove</param>
        public void RemovePostEffect(PostEffect a_postEffect)
        {
            m_postEffects.Remove(a_postEffect);
        }

        /// <summary>
        /// Sets the RenderScale of the DefaultRenderPipeline
        /// </summary>
        /// <param name="a_scale">The scale to set to</param>
        public void SetRenderScale(float a_scale)
        {
            m_renderScale = a_scale;

            Resize(m_width, m_height);
        }

        /// <summary>
        /// Called when the SwapChain is resized
        /// </summary>
        /// <param name="a_width">The new width of the SwapChain.</param>
        /// <param name="a_height">The new height of the SwapChain.</param>
        public override void Resize(uint a_width, uint a_height)
        {
            m_width = a_width;
            m_height = a_height;

            uint scaledWidth = (uint)(m_width * m_renderScale);
            uint scaledHeight = (uint)(m_height * m_renderScale);

            m_depthRenderTexture.Resize(scaledWidth, scaledHeight);

            m_drawRenderTexture.Resize(scaledWidth, scaledHeight);
            m_lightRenderTexture.Resize(scaledWidth, scaledHeight);
            m_forwardRenderTexture.Resize(scaledWidth, scaledHeight);
            m_colorRenderTexture.Resize(m_width, m_height);

            for (uint i = 0; i < PostTextureStackSize; ++i)
            {
                m_postRenderTextures[i].Resize(m_width, m_height);
            }

            SetLightTextures(m_ambientLightMaterial);
            SetLightTextures(m_directionalLightMaterial);
            SetLightTextures(m_pointLightMaterial);
            SetLightTextures(m_spotLightMaterial);

            SetLightTextures(m_directionalLightShadowMaterial);
            SetLightTextures(m_pointLightShadowMaterial);
            SetLightTextures(m_spotLightShadowMaterial);

            SetBlendTextures();

            foreach (PostEffect effect in m_postEffects)
            {
                effect.Resize(m_width, m_height);
            }
        }

        float GetCascade(uint a_index, uint a_count, float a_near, float a_far)
        {
            float endFar = a_far * m_shadowCutoff;

            if (a_index == 0)
            {
                return a_near;
            }
            else if (a_index == a_count)
            {
                return endFar;
            }

            // I believe I need:
            // c = l near (far / near) ^ (i / N) + (1 - l)(near + (i / N)(far - near))
            float fnDiff = endFar - a_near;
            float fON = endFar / a_near;

            float invLambda = 1.0f - m_lambda;

            float cN = (float)a_index / a_count;

            return m_lambda * a_near * Mathf.Pow(fON, cN) + invLambda * (a_near + cN * fnDiff);
        }
        Matrix4 GetCascadeLVPMatrix(float a_near, float a_far, Light a_light, Camera a_camera)
        {
            // I forget everytime so knicked it
            // https://learnopengl.com/Guest-Articles/2021/CSM
            // On second though something did not look right in the maths so I knicked this
            // https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf

            Matrix4 cameraTrans = a_camera.Transform.ToGlobalMatrix();
            Matrix4 proj = a_camera.ToProjection(m_width, m_height, a_near, a_far);
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

            Quaternion rot = a_light.Transform.Rotation;
            rot.W = -rot.W;

            Matrix4 lightTrans = rot.ToMatrix() * new Matrix4(Vector4.UnitX, Vector4.UnitY, Vector4.UnitZ, new Vector4(mid, 1.0f));
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
            Matrix4 lightProj = Matrix4.CreateOrthographic(extents.X, extents.Y, -extents.Z * 2, extents.Z);

            return lightView * lightProj;   
        }

        /// <summary>
        /// Called before starting the shadow pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_lightType">The type of <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow pass is for.</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow pass is for.</param>
        public override void ShadowSetup(LightType a_lightType, Camera a_camera)
        {
            
        }
        /// <summary>
        /// Called before rendering the shadow map for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow map is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow map is for</param>
        /// <param name="a_textureSlot">The slot of the shadow map</param>
        /// <returns>The information for the shadow map</returns>
        public override LightShadowSplit PreShadow(Light a_light, Camera a_camera, uint a_textureSlot) 
        {
            switch (a_light.LightType)
            {
            case LightType.Directional:
            {
                List<IRenderTexture> shadowMaps = new List<IRenderTexture>(a_light.ShadowMaps);
                uint cascadeCount = (uint)shadowMaps.Count;
                float near = a_camera.Near;
                float far = a_camera.Far;

                float cascadeNear = GetCascade(a_textureSlot, cascadeCount, near, far);
                float cascadeFar = GetCascade(a_textureSlot + 1, cascadeCount, near, far);

                Matrix4 lvp = GetCascadeLVPMatrix(cascadeNear, cascadeFar, a_light, a_camera);

                return new LightShadowSplit()
                {
                    LVP = lvp,
                    Split = cascadeFar
                };
            }
            case LightType.Spot:
            {
                SpotLight light = a_light as SpotLight;

                Matrix4 trans = light.Transform.ToGlobalMatrix();
                Matrix4 proj = Matrix4.CreatePerspective(light.OuterCutoffAngle * 2, 1.0f, 0.1f, light.Radius);
                Matrix4 view = Matrix4.Inverse(trans);

                Matrix4 lvp = view * proj;

                return new LightShadowSplit()
                {
                    LVP = lvp,
                    Split = 0.0f
                };
            }
            }
            
            return new LightShadowSplit()
            {
                LVP = Matrix4.Identity,
                Split = 0.0f
            };
        }
        /// <summary>
        /// Called after rendering the shadow map for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow map is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow map is for</param>
        /// <param name="a_textureSlot">The slot of the shadow map</param>
        public override void PostShadow(Light a_light, Camera a_camera, uint a_textureSlot)
        {
            
        }

        /// <summary>
        /// Called before the render pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the render pass is for</param>
        public override void PreRender(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_drawRenderTexture);
        }
        /// <summary>
        /// Called after the render pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the render pass is for.</param>
        public override void PostRender(Camera a_camera)
        {
            
        }

        /// <summary>
        /// Called before the light pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light setup is for</param>
        public override void LightSetup(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_lightRenderTexture);
        }

        /// <summary>
        /// Called before the shadow pass for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow pass is for</param>
        /// <returns>Information to use for the shadow pass</returns>
        public override LightShadowPass PreShadowLight(Light a_light, Camera a_camera)
        {
            LightShadowPass pass = new LightShadowPass();

            switch (a_light.LightType)
            {
            case LightType.Directional:
            {
                List<IRenderTexture> shadowMaps = new List<IRenderTexture>(a_light.ShadowMaps);
                uint cascadeCount = (uint)shadowMaps.Count;

                float near = a_camera.Near;
                float far = a_camera.Far;

                pass.Material = m_directionalLightShadowMaterial;
                pass.Splits = new LightShadowSplit[cascadeCount];

                for (uint i = 0; i < cascadeCount; ++i)
                {
                    float cascadeNear = GetCascade(i, cascadeCount, near, far);
                    float cascadeFar = GetCascade(i + 1, cascadeCount, near, far);

                    Matrix4 lvp = GetCascadeLVPMatrix(cascadeNear, cascadeFar, a_light, a_camera);

                    pass.Splits[i] = new LightShadowSplit()
                    {
                        LVP = lvp,
                        Split = cascadeFar
                    };
                }

                break;
            }
            case LightType.Point:
            {
                pass.Material = m_pointLightShadowMaterial;

                break;
            }
            case LightType.Spot:
            {
                SpotLight light = a_light as SpotLight;

                Matrix4 trans = light.Transform.ToGlobalMatrix();
                Matrix4 proj = Matrix4.CreatePerspective(light.OuterCutoffAngle * 2, 1.0f, 0.1f, light.Radius);
                Matrix4 view = Matrix4.Inverse(trans);

                Matrix4 lvp = view * proj;

                pass.Material = m_spotLightShadowMaterial;
                pass.Splits = new LightShadowSplit[]
                {
                    new LightShadowSplit()
                    {
                        LVP = lvp,
                        Split = 0.0f
                    }    
                };

                break;
            }
            }

            return pass;
        }
        /// <summary>
        /// Called after the shadow pass for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_light">The <see cref="IcarianEngine.Rendering.Lighting.Light" /> the shadow pass is for</param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the shadow pass is for</param>
        public override void PostShadowLight(Light a_light, Camera a_camera)
        {
            
        }
        /// <summary>
        /// Called before the light pass for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> type for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_lightType">The type of <see cref="IcarianEngine.Rendering.Lighting.Light" /></param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light pass is for</param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Material" /> to use for the light pass</returns>
        public override Material PreLight(LightType a_lightType, Camera a_camera)
        {
            switch (a_lightType)
            {
            case LightType.Ambient:
            {
                return m_ambientLightMaterial;
            }
            case LightType.Directional:
            {
                return m_directionalLightMaterial;
            }
            case LightType.Point:
            {
                return m_pointLightMaterial;
            }
            case LightType.Spot:
            {
                return m_spotLightMaterial;
            }
            }   
            
            return null;
        }
        /// <summary>
        /// Called after the light pass for a <see cref="IcarianEngine.Rendering.Lighting.Light" /> type for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_lightType">The type of <see cref="IcarianEngine.Rendering.Lighting.Light" /></param>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the light pass is for.</param>
        public override void PostLight(LightType a_lightType, Camera a_camera)
        {
            
        }

        /// <summary>
        /// Called before the forward pass for a <see cref="IcarianEngine.Rendering.Camera" />
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the forward pass is for</param>
        public override void PreForward(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_forwardRenderTexture, RenderTextureBindMode.ClearColor);
        }
        /// <summary>
        /// Called after the forward pass for a <see cref="IcarianEngine.Rendering.Camera" />
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the forward pass is for</param>
        public override void PostForward(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_colorRenderTexture);
            RenderCommand.BindMaterial(m_blendMaterial);

            RenderCommand.DrawMaterial();
        }

        /// <summary>
        /// Called for the post process pass for a <see cref="IcarianEngine.Rendering.Camera" /> 
        /// </summary>
        /// <param name="a_camera">The <see cref="IcarianEngine.Rendering.Camera" /> the post processing pass is for</param>
        public override void PostProcess(Camera a_camera)
        {
            if (m_postEffects == null)
            {
                RenderCommand.Blit(m_colorRenderTexture, a_camera.RenderTexture);

                return;
            }
            
            int size = m_postEffects.Count;
            if (size == 0)
            {
                RenderCommand.Blit(m_colorRenderTexture, a_camera.RenderTexture);

                return;
            }

            TextureSampler sampler = m_colorSampler;
            uint textureIndex = 0;

            int end = size - 1;
            for (int i = 0; i < size; ++i)
            {
                if (m_postEffects[i].ShouldRun)
                {
                    IRenderTexture renderTexture = m_postRenderTextures[textureIndex];
                    if (i >= end)
                    {
                        renderTexture = a_camera.RenderTexture;
                    }

                    m_postEffects[i].Run(renderTexture, new TextureSampler[] { sampler, m_normalSampler, m_emissionSampler, m_depthSampler });

                    sampler = m_postTextureSamplers[textureIndex];

                    textureIndex = (textureIndex + 1) % 2;
                }
            }
        }

        /// <summary>
        /// Called when the render pipeline is destroyed.
        /// </summary>
        public virtual void Dispose()
        {
            m_drawRenderTexture.Dispose();
            m_lightRenderTexture.Dispose();
            m_forwardRenderTexture.Dispose();
            m_colorRenderTexture.Dispose();

            m_depthRenderTexture.Dispose();

            for (uint i = 0; i < PostTextureStackSize; ++i)
            {
                m_postRenderTextures[i].Dispose();
                m_postTextureSamplers[i].Dispose();
            }

            foreach (PostEffect effect in m_postEffects)
            {
                if (effect is IDestroy dest)
                {
                    if (!dest.IsDisposed)
                    {
                        dest.Dispose();
                    }
                }
                else if (effect is IDisposable disp)
                {
                    disp.Dispose();
                }
            }

            m_defferedColorSampler.Dispose();
            m_normalSampler.Dispose();
            m_specularSampler.Dispose();
            m_emissionSampler.Dispose();
            m_depthSampler.Dispose();
            m_forwardSampler.Dispose();
            m_colorSampler.Dispose();

            m_lightColorSampler.Dispose();

            m_directionalLightMaterial.Dispose();
            m_ambientLightMaterial.Dispose();
            m_pointLightMaterial.Dispose();
            m_spotLightMaterial.Dispose();
            m_directionalLightShadowMaterial.Dispose();
            m_pointLightShadowMaterial.Dispose();
            m_spotLightShadowMaterial.Dispose();
            m_blendMaterial.Dispose();

            m_quadVert.Dispose();

            m_ambientLightPixel.Dispose();
            m_directionalLightPixel.Dispose();
            m_pointLightPixel.Dispose();
            m_spotLightPixel.Dispose();
            m_directionalLightShadowPixel.Dispose();
            m_pointLightShadowPixel.Dispose();
            m_spotLightShadowPixel.Dispose();
            m_blendPixel.Dispose();
        }
    }
}