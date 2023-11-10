using IcarianEngine.Maths;
using IcarianEngine.Rendering.Lighting;
using System;
using System.Collections.Generic;

namespace IcarianEngine.Rendering
{
    public class DefaultRenderPipeline : RenderPipeline, IDisposable
    {
        /// <summary>
        /// The number of cascades to use for directional light shadows.
        /// </summary>
        public const uint CascadeCount = 4;

        VertexShader       m_quadVert;
        PixelShader        m_ambientLightPixel;
        PixelShader        m_directionalLightPixel;
        PixelShader        m_pointLightPixel;
        PixelShader        m_spotLightPixel;
        PixelShader        m_directionalLightShadowPixel;
        PixelShader        m_pointLightShadowPixel;
        PixelShader        m_postPixel;

        Material           m_ambientLightMaterial;
        Material           m_directionalLightMaterial;
        Material           m_pointLightMaterial;
        Material           m_spotLightMaterial;
        Material           m_directionalLightShadowMaterial;
        Material           m_pointLightShadowMaterial;
        Material           m_postMaterial;

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

        float              m_shadowCutoff;
        float              m_lambda;

        /// <summary>
        /// The lambda value for cascaded shadow maps.
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
        /// The shadow cutoff for directional lights.
        /// </summary>
        /// 0-1 range of the far plane to use for shadows.
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
            m_postMaterial.SetTexture(0, m_lightColorSampler);
            m_postMaterial.SetTexture(1, m_normalSampler);
            m_postMaterial.SetTexture(2, m_emissionSampler);
            m_postMaterial.SetTexture(3, m_depthSampler);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="IcarianEngine.Rendering.DefaultRenderPipeline"/> class.
        /// </summary>
        public DefaultRenderPipeline()
        {
            m_lambda = 0.5f;
            m_shadowCutoff = 0.5f;

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

            m_quadVert = VertexShader.LoadVertexShader("[INTERNAL]Quad");

            m_ambientLightPixel = PixelShader.LoadPixelShader("[INTERNAL]AmbientLight");
            m_directionalLightPixel = PixelShader.LoadPixelShader("[INTERNAL]DirectionalLight");
            m_pointLightPixel = PixelShader.LoadPixelShader("[INTERNAL]PointLight");
            m_spotLightPixel = PixelShader.LoadPixelShader("[INTERNAL]SpotLight");
            m_directionalLightShadowPixel = PixelShader.LoadPixelShader("[INTERNAL]DirectionalLightShadow");
            m_pointLightShadowPixel = PixelShader.LoadPixelShader("[INTERNAL]PointLightShadow");
            m_postPixel = PixelShader.LoadPixelShader("[INTERNAL]Post");

            MaterialBuilder ambientLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_ambientLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 5,
                        BufferType = ShaderBufferType.AmbientLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1
                    }
                }
            };

            MaterialBuilder directionalLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_directionalLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 5,
                        BufferType = ShaderBufferType.CameraBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 6,
                        BufferType = ShaderBufferType.SSDirectionalLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 2
                    }
                }
            };
            
            MaterialBuilder pointLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_pointLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 5,
                        BufferType = ShaderBufferType.CameraBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 6,
                        BufferType = ShaderBufferType.SSPointLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 2
                    }
                }
            };

            MaterialBuilder spotLightBuilder = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_spotLightPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 5,
                        BufferType = ShaderBufferType.CameraBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 6,
                        BufferType = ShaderBufferType.SSSpotLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 2
                    }
                }
            };

            m_ambientLightMaterial = Material.CreateMaterial(ambientLightBuilder);
            m_directionalLightMaterial = Material.CreateMaterial(directionalLightBuilder);
            m_pointLightMaterial = Material.CreateMaterial(pointLightBuilder);
            m_spotLightMaterial = Material.CreateMaterial(spotLightBuilder);

            SetTextures(m_ambientLightMaterial);
            SetTextures(m_directionalLightMaterial);
            SetTextures(m_pointLightMaterial);
            SetTextures(m_spotLightMaterial);

            MaterialBuilder directionalLightShadowMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_directionalLightShadowPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 5,
                        BufferType = ShaderBufferType.AShadowTexture2D,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1,
                        Count = (ushort)CascadeCount
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 6,
                        BufferType = ShaderBufferType.CameraBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 2
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 7,
                        BufferType = ShaderBufferType.DirectionalLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 3
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 8,
                        BufferType = ShaderBufferType.SSShadowLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 4
                    }
                }
            };

            MaterialBuilder pointLightShadowMaterail = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_pointLightShadowPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput() 
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput() 
                    {
                        Slot = 5,
                        BufferType = ShaderBufferType.ShadowTextureCube,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 6,
                        BufferType = ShaderBufferType.CameraBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 2
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 7,
                        BufferType = ShaderBufferType.PointLightBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 3
                    },
                }
            };

            m_directionalLightShadowMaterial = Material.CreateMaterial(directionalLightShadowMaterial);
            m_pointLightShadowMaterial = Material.CreateMaterial(pointLightShadowMaterail);

            SetTextures(m_directionalLightShadowMaterial);
            SetTextures(m_pointLightShadowMaterial);

            MaterialBuilder postMaterial = new MaterialBuilder()
            {
                VertexShader = m_quadVert,
                PixelShader = m_postPixel,
                PrimitiveMode = PrimitiveMode.TriangleStrip,
                EnableColorBlending = true,
                ShaderInputs = new ShaderBufferInput[]
                {
                    new ShaderBufferInput()
                    {
                        Slot = 0,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 1,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 2,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 3,
                        BufferType = ShaderBufferType.Texture,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 0
                    },
                    new ShaderBufferInput()
                    {
                        Slot = 4,
                        BufferType = ShaderBufferType.CameraBuffer,
                        ShaderSlot = ShaderSlot.Pixel,
                        Set = 1
                    }
                }
            };

            m_postMaterial = Material.CreateMaterial(postMaterial);

            SetPostTextures();
        }

        /// <summary>
        /// Called when the Swap Chain is resized.
        /// </summary>
        /// <param name="a_width">The new width of the Swap Chain.</param>
        /// <param name="a_height">The new height of the Swap Chain.</param>
        public override void Resize(uint a_width, uint a_height)
        {
            if (a_width == m_width && a_height == m_height)
            {
                return;
            }

            m_width = a_width;
            m_height = a_height;

            m_drawRenderTexture.Resize(m_width, m_height);
            m_lightRenderTexture.Resize(m_width, m_height);

            SetTextures(m_ambientLightMaterial);
            SetTextures(m_directionalLightMaterial);
            SetTextures(m_pointLightMaterial);
            SetTextures(m_spotLightMaterial);

            SetTextures(m_directionalLightShadowMaterial);
            SetTextures(m_pointLightShadowMaterial);

            SetPostTextures();
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

            // I believe I need l near (far / near) ^ (i / N) + (1 - l)(near + (i / N)(far - near))
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

            Matrix4 cameraTrans = a_camera.Transform.ToMatrix();
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
            Quaternion invRot = Quaternion.Inverse(rot);

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
        /// Called before starting the shadow pass.
        /// </summary>
        /// <param name="a_lightType">The type of light the shadow pass is for.</param>
        /// <param name="a_camera">The camera the shadow pass is for.</param>
        public override void ShadowSetup(LightType a_lightType, Camera a_camera)
        {
            
        }
        /// <summary>
        /// Called before rendering the shadow map for a light.
        /// </summary>
        /// <param name="a_light">The light the shadow map is for.</param>
        /// <param name="a_camera">The camera the shadow map is for.</param>
        /// <param name="a_textureSlot">The slot of the shadow map.</param>
        /// <returns>The information for the shadow map.</returns>
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
            }
            
            return new LightShadowSplit()
            {
                LVP = Matrix4.Identity,
                Split = 0.0f
            };
        }
        /// <summary>
        /// Called after rendering the shadow map for a light.
        /// </summary>
        /// <param name="a_light">The light the shadow map is for.</param>
        /// <param name="a_camera">The camera the shadow map is for.</param>
        /// <param name="a_textureSlot">The slot of the shadow map.</param>
        public override void PostShadow(Light a_light, Camera a_camera, uint a_textureSlot)
        {
            
        }

        /// <summary>
        /// Called before the render pass.
        /// </summary>
        /// <param name="a_camera">The camera the render pass is for.</param>
        public override void PreRender(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_drawRenderTexture);
        }
        /// <summary>
        /// Called after the render pass.
        /// </summary>
        /// <param name="a_camera">The camera the render pass is for.</param>
        public override void PostRender(Camera a_camera)
        {
            
        }

        /// <summary>
        /// Called before the light pass.
        /// </summary>
        public override void LightSetup(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(m_lightRenderTexture);
        }

        /// <summary>
        /// Called before the shadow pass for a light.
        /// </summary>
        /// <param name="a_light">The light the shadow pass is for.</param>
        /// <param name="a_camera">The camera the shadow pass is for.</param>
        /// <returns>Information to use for the shadow pass.</returns>
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
            }

            return pass;
        }
        /// <summary>
        /// Called after the shadow pass for a light.
        /// </summary>
        /// <param name="a_light">The light the shadow pass is for.</param>
        /// <param name="a_camera">The camera the shadow pass is for.</param>
        public override void PostShadowLight(Light a_light, Camera a_camera)
        {
            
        }
        /// <summary>
        /// Called before the light pass for a light type.
        /// </summary>
        /// <param name="a_lightType">The type of light.</param>
        /// <param name="a_camera">The camera the light pass is for.</param>
        /// <returns>The material to use for the light pass.</returns>
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
        /// Called after the light pass for a light type.
        /// </summary>
        /// <param name="a_lightType">The type of light.</param>
        /// <param name="a_camera">The camera the light pass is for.</param>
        public override void PostLight(LightType a_lightType, Camera a_camera)
        {
            
        }

        /// <summary>
        /// Called for the post process pass.
        /// </summary>
        public override void PostProcess(Camera a_camera)
        {
            RenderCommand.BindRenderTexture(a_camera.RenderTexture);
            RenderCommand.BindMaterial(m_postMaterial);

            RenderCommand.DrawMaterial();
        }

        /// <summary>
        /// Called when the render pipeline is destroyed.
        /// </summary>
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

            m_directionalLightMaterial.Dispose();
            m_pointLightMaterial.Dispose();
            m_spotLightMaterial.Dispose();
            m_directionalLightShadowMaterial.Dispose();
            m_pointLightShadowMaterial.Dispose();
            m_postMaterial.Dispose();

            m_quadVert.Dispose();
            m_directionalLightPixel.Dispose();
            m_pointLightPixel.Dispose();
            m_spotLightPixel.Dispose();
            m_directionalLightShadowPixel.Dispose();
            m_pointLightShadowPixel.Dispose();
            m_postPixel.Dispose();
        }
    }
}