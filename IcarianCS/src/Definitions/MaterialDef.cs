using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using System;
using System.Collections.Generic;
using System.Reflection;

namespace IcarianEngine.Definitions
{
    public struct UBOField
    {
        /// <summary>
        /// Name of the field in the uniform buffer.
        /// </summary>
        public string Name;
        /// <summary>
        /// Type of the field in the uniform buffer.
        /// </summary>
        public string Value;
    };

    public struct TextureInput
    {
        /// <summary>
        /// Which slot to use for the texture.
        /// </summary>
        public uint Slot;
        /// <summary>
        /// Path relative to the project for the texture file to be used.
        /// </summary>
        [EditorPathString]
        public string Path;
        /// <summary>
        /// How to handle the texture when it is out of bounds.
        /// </summary>
        public TextureAddress AddressMode;
        /// <summary>
        /// How to filter the texture when it is scaled.
        /// </summary>
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
        /// <summary>
        /// Path relative to the project for the vertex shader file to be used.
        /// </summary>
        [EditorTooltip("Path relative to the project for the vertex shader file to be used"), EditorPathString]
        public string VertexShaderPath;
        /// <summary>
        /// Path relative to the project for the pixel shader file to be used.
        /// </summary>
        [EditorTooltip("Path relative to the project for the pixel shader file to be used"), EditorPathString]
        public string PixelShaderPath;
        /// <summary>
        /// Used to determine if it will be rendered by a camera in a matching layer. Bitfield based.
        /// </summary>
        [EditorTooltip("Used to determine if it will be rendered by a camera in a matching layer. Bitfield based")]
        public uint RenderLayer = 0b1;

        /// <summary>
        /// Determine the type of vertex used for the shader.
        /// </summary>
        public Type VertexType = typeof(Vertex);

        /// <summary>
        /// Deterimine vertex data the shader uses for input.
        /// </summary>
        /// When this is null it will use the GetAttributes method on the VertexType to determine the attributes.
        [EditorTooltip("Deterimine vertex data the shader uses for input")]
        public List<VertexInputAttribute> VertexAttributes = null;

        /// <summary>
        /// Which faces to show when rendering.
        /// </summary>
        [EditorTooltip("Which faces to show when rendering")]
        public CullMode CullingMode = CullMode.Back;

        /// <summary>
        /// Which primitive mode to use when rendering.
        /// </summary>
        [EditorTooltip("Which primitive mode to use when rendering")]
        public PrimitiveMode PrimitiveMode = PrimitiveMode.Triangles;

        /// <summary>
        /// The blending mode of the material.
        /// </summary>
        [EditorTooltip("The blending mode of the material")]
        public MaterialBlendMode ColorBlendMode = MaterialBlendMode.None;

        /// <summary>
        /// Texture the material uses.
        /// </summary>
        [EditorTooltip("Textures the material uses")]
        public List<TextureInput> TextureInputs = null;

        /// <summary>
        /// Path relative to the project for the shadow pixel shader file to be used.
        /// </summary>
        [EditorTooltip("Path relative to the project for the shadow vertex shader file to be used"), EditorPathString]
        public string ShadowVertexShaderPath;

        /// <summary>
        /// Type used as the uniform buffer for the shader.
        /// </summary>
        public Type UniformBufferType = null;

        /// <summary>
        /// Used to determine input values for uniform buffers.
        /// </summary>
        [EditorTooltip("Used to determine input values for uniform buffers")]
        public List<UBOField> UniformBufferFields = null;

        /// <summary>
        /// Converts a UBOFieldValue to a string.
        /// </summary>
        public static string UBOFieldValueToString(object a_value)
        {
            if (a_value != null)
            {
                Type type = a_value.GetType();

                if (type == typeof(float))
                {
                    return ((float)a_value).ToString();
                }
                else if (type == typeof(Vector2))
                {
                    Vector2 vector = (Vector2)a_value;

                    return $"{vector.X}, {vector.Y}";
                }
                else if (type == typeof(Vector3))
                {
                    Vector3 vector = (Vector3)a_value;

                    return $"{vector.X}, {vector.Y}, {vector.Z}";
                }
                else if (type == typeof(Vector4))
                {
                    Vector4 vector = (Vector4)a_value;

                    return $"{vector.X}, {vector.Y}, {vector.Z}, {vector.W}";
                }
            }
            else
            {
                Logger.IcarianError("Invalid UBOFieldValue");
            }

            return string.Empty;
        }
        /// <summary>
        /// Converts a string to a UBOFieldValue.
        /// </summary>
        public static object UBOValueToObject(Type a_type, string a_value)
        {
            if (a_type == null)
            {
                Logger.IcarianError("Invalid UniformBufferType");

                return null;
            }

            if (!string.IsNullOrEmpty(a_value))
            {
                if (a_type == typeof(float))
                {
                    return float.Parse(a_value);
                }
                else if (a_type == typeof(Vector2))
                {
                    string[] values = a_value.Split(',');
                    if (values.Length == 2)
                    {
                        return new Vector2(float.Parse(values[0]), float.Parse(values[1]));
                    }
                }
                else if (a_type == typeof(Vector3))
                {
                    string[] values = a_value.Split(',');
                    if (values.Length == 3)
                    {
                        return new Vector3(float.Parse(values[0]), float.Parse(values[1]), float.Parse(values[2]));
                    }
                }
                else if (a_type == typeof(Vector4))
                {
                    string[] values = a_value.Split(',');
                    if (values.Length == 4)
                    {
                        return new Vector4(float.Parse(values[0]), float.Parse(values[1]), float.Parse(values[2]), float.Parse(values[3]));
                    }
                }
            }
            else
            {
                Logger.IcarianError("Material Def Invalid UBOFieldValue");
            }

            return null;
        }

        /// <summary>
        /// Called after the def is loaded to resolve any data.
        /// </summary>
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

            if (UniformBufferType != null && (!UniformBufferType.IsValueType || UniformBufferType.IsEnum))
            {
                Logger.IcarianError("Material Def Invalid UniformBufferType");
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