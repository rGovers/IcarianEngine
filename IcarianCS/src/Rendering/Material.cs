// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;
using IcarianEngine.Rendering.Shaders;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#ifdef ENABLE_STACKTRACE
using System.Diagnostics;
#endif

#include "EngineMaterialInteropStructures.h"

namespace IcarianEngine.Rendering
{
    public struct MaterialBuilder
    {
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Shaders.VertexShader" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        /// Mutually exclusive with <see cref="IcarianEngine.Rendering.Shaders.MeshShader" />
        public VertexShader VertexShader;
        /// <summary>
        /// The <see cref="IcarianEngine.Rendering.Shaders.MeshShader" /> to be used by the <see cref="IcarianEngine.Rendering.Material" />
        /// </summary>
        /// Mutually exclusive with <see cref="IcarianEngine.Rendering.Shaders.VertexShader" />
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
        /// Optional. Must use Shadow Vertex Shader if base Shader is a Vertex Shader.
        public VertexShader ShadowVertexShader;
        /// <summary>
        /// The object used for user UBO variables.
        /// </summary>
        /// Required if the user adds UserUBO to ShaderInputs. 
        /// Must be a struct. 
        /// Mutually exclusive with UBOData
        public object UBOBuffer;
        ///<summary>
        /// The data used for user UBO variables
        /// </summary>
        /// Required if the user adds UserUBO to ShaderInputs. 
        /// Mutually exclusive with UBOObject.
        public byte[] UBOData; 
        /// <summary>
        /// The Array for the user array variables
        /// </summary>
        /// Required if the user adds UserArray to ShaderInputs
        public Array UserArray;
        /// <summary>
        /// The Array data for the user array variables
        /// </summary>
        /// Required if the user adds UserArray to ShaderInputs
        /// Mutually exclusive with UserArray
        public byte[] UserArrayData;
    };

    public class Material : IDestroy
    {
        public delegate void UserArrayCallback();

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram
        (
            uint a_vertexShader,
            uint a_pixelShader,
            ushort a_vertexStride,
            VertexInputAttribute[] a_attributes,
            uint a_cullMode,
            uint a_primitiveMode,
            uint a_colorBlendMode,
            uint a_renderLayer,
            uint a_shadowVertexShader,
            uint a_uboSize,
            IntPtr a_uboBuffer,
            Array a_userArray,
            uint a_arrayStride
        );
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateMeshProgram
        (
            uint a_meshShader,
            uint a_pixelShader,
            ushort a_vertexStride,
            uint a_cullMode,
            uint a_colorBlendMode,
            uint a_renderLayer,
            uint a_uboSize,
            IntPtr a_uboBuffer,
            Array a_userArray,
            uint a_arrayStride
        );
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateComputeProgram(uint a_computeShader, uint a_uboSize, IntPtr a_uboBuffer);
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
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetUserArray(uint a_addr, uint a_elementStride, Array a_array);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetUserArrayCallback(uint a_addr, uint a_elementStride, Array a_array, uint a_callbackAddr);

        static NativeLock s_userArrayLock;
        static List<UserArrayCallback> s_userArrayCallbacks;

        uint        m_bufferAddr = uint.MaxValue;
        Type        m_uboType = null;
        Type        m_uArrayType = null;

        MaterialDef m_def = null;

#ifdef ENABLE_STACKTRACE
        StackTrace  m_stackTrace;
#endif

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
        /// </summary>
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

        internal static void InternalInit()
        {
            s_userArrayCallbacks = new List<UserArrayCallback>();
            s_userArrayLock = new NativeLock();
        }

        internal static void InternalDestroy()
        {
            s_userArrayLock.Dispose();
            s_userArrayCallbacks = null;
        }

        Material(uint a_bufferAddr)
        {
            m_bufferAddr = a_bufferAddr;

#ifdef ENABLE_STACKTRACE
            m_stackTrace = new StackTrace(true);
#endif
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
                Logger.IcarianError($"Invalid UBO data type {type}");

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
        /// Sets the user defined array
        /// </summary>
        /// <param name="a_data">The data to set the user array to</param>
        public void SetUserArray(Array a_data)
        {
            // This whole function is to work around .NET jank with arrays as it does not have fixed/stack arrays until .NET 8 I believe
            // If not for that could just throw an array into a struct and use SetUserUniform
            if (a_data == null)
            {
                Logger.IcarianError("Invalid User Array data");

                return;
            }

            Type type = a_data.GetType();
            Type elementType = type.GetElementType();
            if (elementType != m_uArrayType)
            {
                Logger.IcarianError($"Invalid User Array element type {elementType}");

                return;
            }

            uint stride = (uint)Marshal.SizeOf(elementType);

            // C# is fun with arrays so let the C++ side handle it
            // Cannot easily prepare arrays for the GPU until .NET 8 but Mono is still the best for embedding so we just do not on the C# side
            SetUserArray(m_bufferAddr, stride, a_data);
        }

        static void DispatchUserArrayCallback(uint a_addr)
        {
            if (s_userArrayCallbacks == null)
            {
                Logger.IcarianWarning("User Array Callback invoked after Shutdown");

                return;
            }

            UserArrayCallback callback = null;

            s_userArrayLock.ReadLock();

            try
            {
                uint count = (uint)s_userArrayCallbacks.Count;

                if (a_addr < count)
                {
                    callback = s_userArrayCallbacks[(int)a_addr];
                    s_userArrayCallbacks[(int)a_addr] = null;
                }
            }
            finally
            {
                s_userArrayLock.ReadUnlock();
            }

            if (callback != null)
            {
                callback();
            }
        }

        static uint PushUserArrayCallback(UserArrayCallback a_callback)
        {
            if (s_userArrayCallbacks == null)
            {
                return uint.MaxValue;
            }

            s_userArrayLock.WriteLock();

            try
            {
                uint count = (uint)s_userArrayCallbacks.Count;

                for (uint i = 0; i < count; ++i)
                {
                    if (s_userArrayCallbacks[(int)i] == null)
                    {
                        s_userArrayCallbacks[(int)i] = a_callback;

                        return i;
                    }
                }

                s_userArrayCallbacks.Add(a_callback);

                return count;
            }
            finally
            {
                s_userArrayLock.WriteUnlock();
            }

            Logger.IcarianError("PushUserArrayCallback failed to push callback");

IOP_CSMACRO(pragma warning disable CS0162)

            return uint.MaxValue;

IOP_CSMACRO(pragma warning restore CS0162)
        }

        /// <summary>
        /// Sets the user defined array
        /// </summary>
        /// <param name="a_data">The data to set the user array to</param>
        /// <param name="a_callback">The callback to invoke when the data is set by the render thread</param>
        public void SetUserArray(Array a_data, UserArrayCallback a_callback)
        {
            if (a_data == null)
            {
                Logger.IcarianError("Invalid User Array data");

                return;
            }

            Type type = a_data.GetType();
            Type elementType = type.GetElementType();
            if (elementType != m_uArrayType)
            {
                Logger.IcarianError($"Invalid User Array element type {elementType}");

                return;
            }

            // Urgh this is annoying
            // So was digging around in C# and delegates are a pain in the ass
            // Yes there is ways to create a unmanaged function pointer and also a way to invoke delegates
            // HOWEVER that delegate still has to be pinned/alive otherwise the GC will go fuck you
            // That completely eliminates the whole point of being able to create a function pointer
            // To get around that we just store the delegate on the C# side and give the C++ side a handle
            // Fuck garbage collected languages and relocation
            uint stride = (uint)Marshal.SizeOf(elementType);
            uint callbackAddr = uint.MaxValue;
            if (a_callback != null)
            {
                callbackAddr = PushUserArrayCallback(a_callback);
            }

            SetUserArrayCallback(m_bufferAddr, stride, a_data, callbackAddr);
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

            if (a_builder.UBOBuffer != null && a_builder.UBOData != null)
            {
                Logger.IcarianError("Material both UBOBuffer and UBOData");

                return null;
            }

            if (a_builder.UserArray != null && a_builder.UserArrayData != null)
            {
                Logger.IcarianError("Material both UserArray and UserArrayData");

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
            else if (a_builder.UBOData != null)
            {
                uboSize = (uint)a_builder.UBOData.Length;
                uboBuffer = Marshal.AllocHGlobal((int)uboSize);
                Marshal.Copy(a_builder.UBOData, 0, uboBuffer, (int)uboSize);
            }

            Type elementType = null;
            uint arrayStride = 0;
            Array userArrayData = null;
            if (a_builder.UserArray != null)
            {
                Type type = a_builder.UserArray.GetType();
                elementType = type.GetElementType();
                arrayStride = (uint)Marshal.SizeOf(elementType);
                userArrayData = a_builder.UserArray;
            }
            else if (a_builder.UserArrayData != null)
            {
                arrayStride = 1;
                userArrayData = a_builder.UserArrayData;
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
                    uboBuffer,
                    userArrayData,
                    arrayStride
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
                    uboSize,
                    uboBuffer,
                    userArrayData,
                    arrayStride
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

            mat.m_uArrayType = elementType;
            if (a_builder.UBOBuffer != null)
            {
                mat.m_uboType = a_builder.UBOBuffer.GetType();
            }

            return mat;
        }

        /// <summary>
        /// Creates a Material from a <see cref="IcarianEngine.Rendering.Shaders.ComputeShader" />
        /// </summary>
        /// <param name="a_shader">The <see cref="IcarianEngine.Rendering.Shaders.ComputeShader" /> to use</param>
        /// <param name="a_userUBO">The value to set the UBO to</param>
        /// <returns>The created Material. Returns null when invalid.</returns>
        public static Material CreateMaterial(ComputeShader a_shader, object a_userUBO)
        {
            if (a_shader == null || a_shader.IsDisposed)
            {
                Logger.IcarianWarning("Creating Material with null ComputeShader");

                return null;
            }

            if (a_shader.ComputeMode != ComputeMode.Graphics)
            {
                Logger.IcarianError("Creating Compute Material with non Graphics Compute Shader");

                return null;
            }

            uint uboSize = 0;
            IntPtr uboBuffer = IntPtr.Zero;
            if (a_userUBO != null)
            {
                uboSize = (uint)Marshal.SizeOf(a_userUBO);
                uboBuffer = Marshal.AllocHGlobal((int)uboSize);
                Marshal.StructureToPtr(a_userUBO, uboBuffer, false);
            }

            uint addr = GenerateComputeProgram(a_shader.InternalAddr, uboSize, uboBuffer);

            // Trust the GC bout as far as I can throw it
            // Therefore memory stays in the C# domain
            if (uboBuffer != IntPtr.Zero)
            {
                Marshal.FreeHGlobal(uboBuffer);
            }

            if (addr == uint.MaxValue)
            {
                Logger.IcarianError("Failed to create Compute Material");

                return null;
            }

            Material mat = new Material(addr);

            if (a_userUBO != null)
            {
                mat.m_uboType = a_userUBO.GetType();
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
            bool vertexSet = !string.IsNullOrWhiteSpace(a_def.VertexShaderPath);
            bool meshSet = !string.IsNullOrWhiteSpace(a_def.MeshShaderPath);
            if (!vertexSet && !meshSet)
            {
                Logger.IcarianError("Material invalid Vertex/Mesh shader path");

                return null;
            }

            if (vertexSet && meshSet)
            {
                Logger.IcarianError("Material both Vertex and Mesh shader set");

                return null;
            }

            if (string.IsNullOrWhiteSpace(a_def.PixelShaderPath))
            {
                Logger.IcarianError("Material invalid Pixel shader path");

                return null;
            }

            if (a_def.VertexType == null)
            {
                Logger.IcarianError("Material no vertex type");

                return null;
            }

            VertexInputAttribute[] vertexInputAttributes = null;
            if (a_def.VertexAttributes != null)
            {
                vertexInputAttributes = a_def.VertexAttributes.ToArray();
            }

            VertexShader vertexShader = null;
            if (vertexSet)
            {
                vertexShader = AssetLibrary.LoadVertexShader(a_def.VertexShaderPath);

                if (vertexShader == null)
                {
                    Logger.IcarianError("Material invalid Vertex shader");

                    return null;
                }
            }

            MeshShader meshShader = null;
            if (meshSet)
            {
                meshShader = AssetLibrary.LoadMeshShader(a_def.MeshShaderPath);

                if (meshShader == null)
                {
                    Logger.IcarianError("Material invalid Mesh shader");

                    return null;
                }
            }

            PixelShader pixelShader = AssetLibrary.LoadPixelShader(a_def.PixelShaderPath);
            if (pixelShader == null)
            {
                Logger.IcarianError("Material invalid Pixel shader");

                return null;
            }

            VertexShader shadowVertexShader = null;
            if (!Application.IsEditor && !string.IsNullOrEmpty(a_def.ShadowVertexShaderPath))
            {
                shadowVertexShader = AssetLibrary.LoadVertexShader(a_def.ShadowVertexShaderPath);
                if (shadowVertexShader == null)
                {
                    Logger.IcarianError("Material invalid shadow Vertex shader");

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
                MeshShader = meshShader,
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
        /// Called when the Material is being Disposed/Finalised
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
                    Logger.IcarianError("Material not Disposed");

#ifdef ENABLE_STACKTRACE
                    CallStack.PrintStackTrace(m_stackTrace);
#endif
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("Material already Disposed");
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
// Copyright (c) 2025 River Govers
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
