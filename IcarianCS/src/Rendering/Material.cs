// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Rendering.Shaders;
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
        /// The <see cref="IcarianEngine.Rendering.VertexShader" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        /// Mutually exclusive with MeshShader
        public VertexShader VertexShader;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.MeshShader" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        /// Mutally exclusive with VertexShader
        public MeshShader MeshShader;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.PixelShader" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        public PixelShader PixelShader;
        /// <summary>
        /// The stride between each Vertex.
        /// </summary>
        public ushort VertexStride; 
        /// <summary>
        /// The attributes of the Vertex type when using <see cref="IcarianEngine.Rendering.VertexShader" />
        /// </summary>
        public VertexInputAttribute[] Attributes;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.CullMode" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        public CullMode CullingMode;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.PrimitiveMode" /> when using <see cref="IcarianEngine.Rendering.VertexShader" />
        /// </summary>
        public PrimitiveMode PrimitiveMode;
        /// <summary>
        /// The render layer to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        public uint RenderLayer;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.MaterialBlendMode" /> of the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        public MaterialBlendMode ColorBlendMode;
        /// <summary>
        /// The shadow <see cref="IcarianEngine.Rendering.VertexShader" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        /// Optional
        public VertexShader ShadowVertexShader;
        /// <summary>
        /// The object used to for user UBO variables.
        /// </summary>
        /// Required if the user adds UserUBO to ShaderInputs. Must be a struct.
        public object UBOBuffer;
    };

    public class Material : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram(uint a_vertexShader, uint a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, uint a_cullMode, uint a_primitiveMode, uint a_colorBlendMode, uint a_renderLayer, uint a_shadowVertexShader, uint a_uboSize, IntPtr a_uboBuffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateMeshProgram(uint a_meshShader, uint a_pixelShader, ushort a_vertexStride, uint a_cullMode, uint a_colorBlendMode, uint a_renderLayer, uint a_shadowVertexShader, uint a_uboSize, IntPtr a_uboBuffer);
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

        uint        m_bufferAddr = uint.MaxValue;
        Type        m_uboType = null;
        MaterialDef m_def = null;

        /// <summary>
        /// Determines if the Material has been Disposed/Finalised
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
        /// The render layer the Material is on
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
        /// Gets the <see cref="IcarianEngine.Rendering.MaterialMode" /> of the Material
        public MaterialMode MaterialMode
        {
            get
            {
                RenderProgram val = GetProgramBuffer(m_bufferAddr);

                return val.MaterialMode;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.Definitions.MaterialDef" /> used to create the Material
        /// </summary>
        public MaterialDef Def
        {
            get
            {
                return m_def;
            }
        }

        Material(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;
        }

        /// <summary>
        /// Sets the user defined uniform buffer object
        /// </summary>
        /// <param name="a_data">The data to set the uniform buffer object to</param>
        /// Must be a struct. Must match the type used to create the Material
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
        /// Sets the <see cref="IcarianEngine.Rendering.TextureSampler" /> for the Material
        /// </summary>
        /// <param name="a_shaderSlot">The slot to set the <see cref="IcarianEngine.Rendering.TextureSampler" /> to</param>
        /// <param name="a_sampler">The <see cref="IcarianEngine.Rendering.TextureSampler" /> to use</param>
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
        /// Creates a Material from a <see cref="IcarianEngine.Rendering.MaterialBuilder" />
        /// </summary>
        /// <param name="a_builder">The <see cref="IcarianEngine.Rendering.MaterialBuilder" /> to use</param>
        /// <returns>The created Material. Returns null when invalid.</returns>
        public static Material CreateMaterial(MaterialBuilder a_builder)
        {
            if (a_builder.VertexShader == null && a_builder.MeshShader == null)
            {
                Logger.IcarianError("Material invalid Vertex/Mesh shader");

                return null;
            }

            if (a_builder.VertexShader != null && a_builder.MeshShader != null)
            {
                Logger.IcarianError("Material both Mesh and Vertex shader");

                return null;
            }

            if (a_builder.PixelShader == null)
            {
                Logger.IcarianError("Material invalid Pixel shader");

                return null;
            }

            uint shadowVertexShader = uint.MaxValue;
            if (a_builder.ShadowVertexShader != null)
            {
                shadowVertexShader = a_builder.ShadowVertexShader.InternalAddr;
            }

            uint uboSize = 0;
            IntPtr uboBuffer = IntPtr.Zero;
            if (a_builder.UBOBuffer != null)
            {
                uboSize = (uint)Marshal.SizeOf(a_builder.UBOBuffer);
                uboBuffer = Marshal.AllocHGlobal((int)uboSize);
                Marshal.StructureToPtr(a_builder.UBOBuffer, uboBuffer, false);
            }

            uint bufferAddr = uint.MaxValue;
            if (a_builder.VertexShader != null)
            {
                bufferAddr = GenerateProgram
                (
                    a_builder.VertexShader.InternalAddr, 
                    a_builder.PixelShader.InternalAddr, 
                    a_builder.VertexStride, 
                    a_builder.Attributes, 
                    (uint)a_builder.CullingMode, 
                    (uint)a_builder.PrimitiveMode, 
                    (uint)a_builder.ColorBlendMode, 
                    a_builder.RenderLayer,
                    shadowVertexShader, 
                    uboSize,
                    uboBuffer
                );
            }
            else
            {
                bufferAddr = GenerateMeshProgram
                (
                    a_builder.MeshShader.InternalAddr,
                    a_builder.PixelShader.InternalAddr,
                    a_builder.VertexStride,
                    (uint)a_builder.CullingMode,
                    (uint)a_builder.ColorBlendMode,
                    a_builder.RenderLayer,
                    shadowVertexShader,
                    uboSize,
                    uboBuffer
                );
            }

            // Trust the GC bout as far as I can throw it
            // Therefore memory stays in the C# domain
            if (uboBuffer != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(uboBuffer);
            }

            if (bufferAddr == uint.MaxValue)
            {
                Logger.IcarianError("Failed to create Material");

                return null;
            }

            Material mat = new Material(bufferAddr);

            if (a_builder.UBOBuffer != null)
            {
                mat.m_uboType = a_builder.UBOBuffer.GetType();
            }

            return mat;
        }

        /// <summary>
        /// Creates a Material from a <see cref="IcarianEngine.Definitions.MaterialDef" />
        /// </summary>
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.MaterialDef" /> to use</param>
        /// <returns>The created Material. Returns null when invalid.</returns>
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
            if (!Application.IsEditor && !string.IsNullOrEmpty(a_def.ShadowVertexShaderPath))
            {
                shadowVertexShader = AssetLibrary.LoadVertexShader(a_def.ShadowVertexShaderPath);
                if (shadowVertexShader == null)
                {
                    Logger.IcarianError("Material invalid shadow vertex shader");

                    return null;
                }
            }

            object userUBO = null;
            Type t = a_def.UniformBufferType;
            if (t != null)
            {
                userUBO = Activator.CreateInstance(t);

                if (a_def.UniformBufferFields != null)
                {
                    foreach (UBOField field in a_def.UniformBufferFields)
                    {
                        FieldInfo info = t.GetField(field.Name);
                        if (info == null)
                        {
                            continue;
                        }

                        object value = MaterialDef.UBOValueToObject(info.FieldType, field.Value);

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
                CullingMode = a_def.CullingMode,
                PrimitiveMode = a_def.PrimitiveMode,
                ColorBlendMode = a_def.ColorBlendMode,
                RenderLayer = a_def.RenderLayer,
                ShadowVertexShader = shadowVertexShader,
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
        /// Disposes the Material
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the material is being Disposed/Finalised
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
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