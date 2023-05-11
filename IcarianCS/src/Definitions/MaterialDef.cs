using System;
using System.Collections.Generic;
using System.Reflection;
using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public struct TextureInput
    {
        public uint Slot;
        public string Path;
        public TextureAddress AddressMode;
        public TextureFilter FilterMode;

        public override int GetHashCode()
        {
            unchecked
            {
                // Do not need the slot for the hash probably not the best idea but doing anyway
                int hash = 73;
                hash = hash * 79 + Path.GetHashCode();
                hash = hash * 79 + AddressMode.GetHashCode();
                hash = hash * 79 + FilterMode.GetHashCode();
                return hash;
            }
        }
    }

    public class MaterialDef : Def
    {
        [EditorTooltip("Path relative to the project for the vertex shader file to be used.")]
        public string VertexShaderPath;
        [EditorTooltip("Path relative to the project for the pixel shader file to be used.")]
        public string PixelShaderPath;
        [EditorTooltip("Used to determine if it will be rendered by a camera in a matching layer. Binary bit based.")]
        public uint RenderLayer = 0b1;

        public Type VertexType = typeof(Vertex);

        [EditorTooltip("Deterimine vertex data the shader uses for input.")]
        public List<VertexInputAttribute> VertexAttributes = null;
        
        [EditorTooltip("Used to determine input values for shaders.")]
        public List<ShaderBufferInput> ShaderBuffers = null;

        [EditorTooltip("Which faces to show when rendering.")]
        public CullMode CullingMode = CullMode.Back;

        public PrimitiveMode PrimitiveMode = PrimitiveMode.Triangles;

        [EditorTooltip("Enables color blending.")]
        public bool EnableColorBlending = false;

        [EditorTooltip("Textures the material uses.")]
        public List<TextureInput> TextureInputs = null;

        public override void PostResolve()
        {
            base.PostResolve();

            if (VertexType == null)
            {
                Logger.IcarianError("Material Def Invalid VertexType");

                return;
            }

            if (VertexAttributes != null && VertexAttributes.Count > 0)
            {
                return;
            }

            MethodInfo methodInfo = VertexType.GetMethod("GetAttributes", BindingFlags.Public | BindingFlags.Static);
            if (methodInfo == null)
            {
                Logger.IcarianError("Material Def no VertexAttributes");

                return;
            }

            VertexAttributes = new List<VertexInputAttribute>(methodInfo.Invoke(null, null) as VertexInputAttribute[]);
        }
    }
}