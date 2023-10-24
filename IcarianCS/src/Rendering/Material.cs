using IcarianEngine.Definitions;
using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineMaterialInteropStructures.h"

namespace IcarianEngine.Rendering
{
    public struct MaterialBuilder
    {
        /// <summary>
        /// The vertex shader to be used by the material.
        /// </summary>
        public VertexShader VertexShader;
        /// <summary>
        /// The pixel shader to be used by the material.
        /// </summary>
        public PixelShader PixelShader;
        /// <summary>
        /// The stride between each vertex.
        /// </summary>
        public ushort VertexStride; 
        /// <summary>
        /// The attributes of the vertex type.
        /// </summary>
        public VertexInputAttribute[] Attributes;
        /// <summary>
        /// The shader inputs to be used by the material.
        /// </summary>
        public ShaderBufferInput[] ShaderInputs;
        /// <summary>
        /// The culling mode to be used by the material.
        /// </summary>
        public CullMode CullingMode;
        /// <summary>
        /// The primitive mode to be used by the material.
        /// </summary>
        public PrimitiveMode PrimitiveMode;
        /// <summary>
        /// The render layer to be used by the material.
        /// </summary>
        public uint RenderLayer;
        /// <summary>
        /// Enables color blending.
        /// </summary>
        public bool EnableColorBlending;
        /// <summary>
        /// The shadow vertex shader to be used by the material.
        /// </summary>
        /// Optional
        public VertexShader ShadowVertexShader;
        /// <summary>
        /// The shadow shader inputs to be used by the material.
        /// </summary>
        /// Needed if ShadowVertexShader is not null
        public ShaderBufferInput[] ShadowShaderInputs;
        /// <summary>
        /// The object used to for user UBO variables.
        /// </summary>
        /// Required if the user adds UserUBO to ShaderInputs. Must be a struct.
        public object UBOBuffer;
    };

    public class Material : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateInternalProgram(uint a_renderProgram);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram(uint a_vertexShader, uint a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs, uint a_cullMode, uint a_primitiveMode, uint a_enableColorBlending, uint a_renderLayer, uint a_shadowVertexShader, ShaderBufferInput[] a_shadowShaderInputs, uint a_uboSize, IntPtr a_uboBuffer); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static RenderProgram GetProgramBuffer(uint a_addr); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetProgramBuffer(uint a_addr, RenderProgram a_program);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyProgram(uint a_addr); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetTexture(uint a_addr, uint a_shaderSlot, uint a_samplerAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetUserUniform(uint a_addr, uint a_uboSize, IntPtr a_uboBuffer);

        /// <summary>
        /// The directional light material used by the default render pipeline.
        /// </summary>
        public static Material DirectionalLightMaterial = null;
        /// <summary>
        /// The point light material used by the default render pipeline.
        /// </summary>
        public static Material PointLightMaterial = null;
        /// <summary>
        /// The spot light material used by the default render pipeline.
        /// </summary>
        public static Material SpotLightMaterial = null;
        /// <summary>
        /// The post processing material used by the default render pipeline.
        /// </summary>
        public static Material PostMaterial = null;

        uint        m_bufferAddr = uint.MaxValue;
        Type        m_uboType = null;
        MaterialDef m_def = null;

        /// <summary>
        /// Determines if the material has been disposed.
        /// </summary>
        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        /// <summary>
        /// The render layer the material is on.
        /// </summary>
        /// When a bit matches the render layer of the render source it will be rendered.
        public uint RenderLayer
        {
            get
            {
                return GetProgramBuffer(m_bufferAddr).RenderLayer;
            }
            set
            {
                RenderProgram val = GetProgramBuffer(m_bufferAddr);

                val.RenderLayer = value;

                SetProgramBuffer(m_bufferAddr, val);
            }
        }
        
        /// <summary>
        /// The material definition used to create the material.
        /// </summary>
        public MaterialDef Def
        {
            get
            {
                return m_def;
            }
        }

        internal static void Init()
        {
            DirectionalLightMaterial = new Material(InternalRenderProgram.DirectionalLight);
            PointLightMaterial = new Material(InternalRenderProgram.PointLight);
            SpotLightMaterial = new Material(InternalRenderProgram.SpotLight);
            PostMaterial = new Material(InternalRenderProgram.Post);
        }
        internal static void Destroy()
        {
            DirectionalLightMaterial.Dispose();
            PointLightMaterial.Dispose();
            SpotLightMaterial.Dispose();
            PostMaterial.Dispose();
        }

        Material(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;
        }
        Material(InternalRenderProgram a_program)
        {
            m_bufferAddr = GenerateInternalProgram((uint)a_program);

            RenderLayer = 0b1;
        }

        /// <summary>
        /// Sets the user defined uniform buffer object.
        /// </summary>
        /// <param name="a_data">The data to set the uniform buffer object to.</param>
        /// Must be a struct. Must match the type used to create the material.
        public void SetUserUniform(object a_data)
        {
            if (a_data == null)
            {
                Logger.IcarianError("Invalid UBO data");

                return;
            }


            Type type = a_data.GetType();
            if (type != m_uboType)
            {
                Logger.IcarianError("Invalid UBO data type");

                return;
            }
            
            // Trust the GC bout as far as I can throw it
            uint uboSize = (uint)Marshal.SizeOf(a_data);
            IntPtr uboBuffer = Marshal.AllocHGlobal((int)uboSize);

            Marshal.StructureToPtr(a_data, uboBuffer, false);

            SetUserUniform(m_bufferAddr, uboSize, uboBuffer);

            Marshal.FreeHGlobal(uboBuffer);
        }

        /// <summary>
        /// Sets the texture sampler for the material.
        /// </summary>
        /// <param name="a_shaderSlot">The slot to set the texture sampler to.</param>
        /// <param name="a_sampler">The texture sampler to use.</param>
        public void SetTexture(uint a_shaderSlot, TextureSampler a_sampler)
        {
            if (a_sampler != null)
            {
                SetTexture(m_bufferAddr, a_shaderSlot, a_sampler.BufferAddr);
            }
            else
            {
                Logger.IcarianError("Invalid Sampler");
            }
        }

        /// <summary>
        /// Creates a material from a material builder.
        /// </summary>
        /// <param name="a_builder">The material builder to use.</param>
        /// <returns>The created material. Returns null when invalid.</returns>
        public static Material CreateMaterial(MaterialBuilder a_builder)
        {
            if (a_builder.VertexShader == null)
            {
                Logger.IcarianError("Material invalid vertex shader");

                return null;
            }

            if (a_builder.PixelShader == null)
            {
                Logger.IcarianError("Material invalid pixel shader");

                return null;
            }

            if (a_builder.Attributes == null)
            {
                Logger.IcarianError("Material invalid vertex attributes");

                return null;
            }

            if (a_builder.ShaderInputs == null)
            {
                Logger.IcarianError("Material invalid shader inputs");

                return null;
            }

            if (a_builder.ShadowVertexShader != null && a_builder.ShadowShaderInputs == null)
            {
                Logger.IcarianError("Material invalid shadow shader inputs");

                return null;
            }

            uint shadowVertexShader = uint.MaxValue;
            ShaderBufferInput[] shadowShaderInputs = null;
            if (a_builder.ShadowVertexShader != null)
            {
                shadowVertexShader = a_builder.ShadowVertexShader.InternalAddr;
                shadowShaderInputs = a_builder.ShadowShaderInputs;
            }

            uint enableColorBlending = 0;
            if (a_builder.EnableColorBlending)
            {
                enableColorBlending = 1;
            }

            uint uboSize = 0;
            IntPtr uboBuffer = IntPtr.Zero;
            if (a_builder.UBOBuffer != null)
            {
                uboSize = (uint)Marshal.SizeOf(a_builder.UBOBuffer);
                uboBuffer = Marshal.AllocHGlobal((int)uboSize);
                Marshal.StructureToPtr(a_builder.UBOBuffer, uboBuffer, false);
            }

            uint bufferAddr = GenerateProgram
            (
                a_builder.VertexShader.InternalAddr, 
                a_builder.PixelShader.InternalAddr, 
                a_builder.VertexStride, 
                a_builder.Attributes, 
                a_builder.ShaderInputs, 
                (uint)a_builder.CullingMode, 
                (uint)a_builder.PrimitiveMode, 
                enableColorBlending, 
                a_builder.RenderLayer,
                shadowVertexShader, 
                shadowShaderInputs,
                uboSize,
                uboBuffer
            );

            // Trust the GC bout as far as I can throw it
            // Therefore memory stays in the C# domain
            if (uboBuffer != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(uboBuffer);
            }

            Material mat = new Material(bufferAddr);

            if (a_builder.UBOBuffer != null)
            {
                mat.m_uboType = a_builder.UBOBuffer.GetType();
            }

            return mat;
        }

        /// <summary>
        /// Creates a material from a material definition.
        /// </summary>
        /// <param name="a_def">The material definition to use.</param>
        /// <returns>The created material. Returns null when invalid.</returns>
        /// @see IcarianEngine.AssetLibrary.GetMaterial
        public static Material FromDef(MaterialDef a_def)
        {
            if (string.IsNullOrWhiteSpace(a_def.PixelShaderPath) || string.IsNullOrWhiteSpace(a_def.VertexShaderPath))
            {
                Logger.IcarianWarning("Material invalid shader path");

                return null;
            }

            if (a_def.VertexType == null)
            {
                Logger.IcarianWarning("Material no vertex type");

                return null;
            }

            ShaderBufferInput[] shaderInput = null;
            if (a_def.ShaderBuffers != null)
            {
                shaderInput = a_def.ShaderBuffers.ToArray();
            }

            VertexInputAttribute[] vertexInputAttributes = null;
            if (a_def.VertexAttributes != null)
            {
                vertexInputAttributes = a_def.VertexAttributes.ToArray();
            }

            VertexShader vertexShader = AssetLibrary.LoadVertexShader(a_def.VertexShaderPath);
            if (vertexShader == null)
            {
                Logger.IcarianError("Material invalid vertex shader");

                return null;
            }

            PixelShader pixelShader = AssetLibrary.LoadPixelShader(a_def.PixelShaderPath);
            if (pixelShader == null)
            {
                Logger.IcarianError("Material invalid pixel shader");

                return null;
            }

            VertexShader shadowVertexShader = null;
            ShaderBufferInput[] shadowShaderInput = null;
            if (!string.IsNullOrEmpty(a_def.ShadowVertexShaderPath))
            {
                shadowVertexShader = AssetLibrary.LoadVertexShader(a_def.ShadowVertexShaderPath);
                if (shadowVertexShader == null)
                {
                    Logger.IcarianError("Material invalid shadow vertex shader");

                    return null;
                }

                if (a_def.ShadowShaderBuffers != null)
                {
                    shadowShaderInput = a_def.ShadowShaderBuffers.ToArray();
                }
            }

            object userUBO = null;
            Type t = a_def.UniformBufferType;
            if (t != null)
            {
                userUBO = Activator.CreateInstance(t);

                foreach (UBOField field in a_def.UniformBufferFields)
                {
                    FieldInfo info = t.GetField(field.Name);
                    if (info != null)
                    {
                        Type fieldType = info.FieldType;

                        object value = MaterialDef.UBOValueToObject(fieldType, field.Value);

                        info.SetValue(userUBO, value);
                    }
                }
            }

            MaterialBuilder materialBuilder = new MaterialBuilder()
            {
                VertexShader = vertexShader,
                PixelShader = pixelShader,
                VertexStride = (ushort)Marshal.SizeOf(a_def.VertexType),
                Attributes = vertexInputAttributes,
                ShaderInputs = shaderInput,
                CullingMode = a_def.CullingMode,
                PrimitiveMode = a_def.PrimitiveMode,
                EnableColorBlending = a_def.EnableColorBlending,
                RenderLayer = a_def.RenderLayer,
                ShadowVertexShader = shadowVertexShader,
                ShadowShaderInputs = shadowShaderInput,
                UBOBuffer = userUBO
            };

            Material mat = CreateMaterial(materialBuilder);
            mat.m_def = a_def;

            if (a_def.TextureInputs != null)
            {
                foreach (TextureInput texInput in a_def.TextureInputs)
                {
                    TextureSampler sampler = AssetLibrary.GetSampler(texInput);
                    if (sampler == null)
                    {
                        Logger.IcarianWarning("Material invalid sampler");

                        return mat;
                    }

                    mat.SetTexture(texInput.Slot, sampler);
                }
            }
            
            return mat; 
        }

        /// <summary>
        /// Disposes the material.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the material is being disposed.
        /// </summary>
        /// <param name="a_disposing">Determines if the material is being disposed.</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyProgram(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Material Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Material Dispose");
            }
        }

        ~Material()
        {
            Dispose(false);
        }
    }
}
