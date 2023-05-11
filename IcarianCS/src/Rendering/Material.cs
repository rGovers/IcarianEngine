using IcarianEngine.Definitions;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct RenderProgram
    {
        public uint VertexShader;
        public uint PixelShader;
        public uint RenderLayer;
        public ushort VertexStride;
        public ushort VertexInputCount;
        public IntPtr VertexInputAttributes;
        public ushort ShaderBufferInputCount;
        public IntPtr ShaderBufferInputs;
        public CullMode CullingMode;
        public PrimitiveMode PrimitiveMode;
        public byte ColorBlendEnabled;
        IntPtr Data;
        byte Flags;
    };

    public enum ShaderBufferType : ushort
    {
        Null = ushort.MaxValue,
        CameraBuffer = 0,
        ModelBuffer = 1,
        DirectionalLightBuffer = 2,
        PointLightBuffer = 3,
        SpotLightBuffer = 4,
        Texture = 5,
        PushTexture = 6
    };

    public enum ShaderSlot : ushort
    {
        Null = ushort.MaxValue,
        Vertex = 0,
        Pixel = 1,
        All = 2
    };

    public enum CullMode : ushort
    {
        None = 0,
        Front = 1,
        Back = 2,
        Both = 3
    };
    public enum PrimitiveMode : ushort
    {
        Triangles = 0,
        TriangleStrip = 1
    };

    internal enum InternalRenderProgram : ushort
    {
        DirectionalLight = 0,
        PointLight = 1,
        SpotLight = 2,
        Post = 3
    }

    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    public struct ShaderBufferInput
    {   
        public ushort Slot;
        public ShaderBufferType BufferType;
        public ShaderSlot ShaderSlot;
        public ushort Set;
    };

    public class Material : IDestroy
    {
        public static Material DirectionalLightMaterial = null;
        public static Material PointLightMaterial = null;
        public static Material SpotLightMaterial = null;
        public static Material PostMaterial = null;

        MaterialDef m_def = null;

        uint        m_bufferAddr = uint.MaxValue;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateInternalProgram(InternalRenderProgram a_renderProgram);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateProgram(uint a_vertexShader, uint a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs, uint a_cullMode, uint a_primitiveMode, uint a_enableColorBlending); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static RenderProgram GetProgramBuffer(uint a_addr); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetProgramBuffer(uint a_addr, RenderProgram a_program);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyProgram(uint a_addr); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetTexture(uint a_addr, uint a_shaderSlot, uint a_samplerAddr);

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

        Material(InternalRenderProgram a_program)
        {
            m_bufferAddr = GenerateInternalProgram(a_program);

            RenderLayer = 0b1;
        }
        public Material(VertexShader a_vertexShader, PixelShader a_pixelShader, ushort a_vertexStride, VertexInputAttribute[] a_attributes, ShaderBufferInput[] a_shaderInputs, CullMode a_cullMode = CullMode.Back, PrimitiveMode a_primitiveMode = PrimitiveMode.Triangles, bool a_enableColorBlending = false)
        {
            if (a_enableColorBlending)
            {
                m_bufferAddr = GenerateProgram(a_vertexShader.InternalAddr, a_pixelShader.InternalAddr, a_vertexStride, a_attributes, a_shaderInputs, (uint)a_cullMode, (uint)a_primitiveMode, 1);
            }
            else
            {
                m_bufferAddr = GenerateProgram(a_vertexShader.InternalAddr, a_pixelShader.InternalAddr, a_vertexStride, a_attributes, a_shaderInputs, (uint)a_cullMode, (uint)a_primitiveMode, 0);
            }

            RenderLayer = 0b1;
        }

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

            Material mat = new Material(vertexShader, pixelShader, (ushort)Marshal.SizeOf(a_def.VertexType), vertexInputAttributes, shaderInput, a_def.CullingMode, a_def.PrimitiveMode, a_def.EnableColorBlending)
            {
                m_def = a_def,
                RenderLayer = a_def.RenderLayer
            };

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

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

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
    }}