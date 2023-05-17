using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.UI;
using System;
using System.Collections.Concurrent;

namespace IcarianEngine
{
    public static class AssetLibrary
    {
        static ConcurrentDictionary<string, Material>             s_materials;
        static ConcurrentDictionary<string, VertexShader>         s_vertexShaders;
        static ConcurrentDictionary<string, PixelShader>          s_pixelShaders;

        static ConcurrentDictionary<string, Texture>              s_textures;
        static ConcurrentDictionary<TextureInput, TextureSampler> s_textureSamplers;

        static ConcurrentDictionary<string, Model>                s_models;
     
        static ConcurrentDictionary<string, Font>                 s_fonts;

        static ConcurrentDictionary<string, CollisionShape>       s_collisionShapes;

        internal static void Init()
        {
            s_materials = new ConcurrentDictionary<string, Material>();

            s_vertexShaders = new ConcurrentDictionary<string, VertexShader>();
            s_pixelShaders = new ConcurrentDictionary<string, PixelShader>();

            s_textures = new ConcurrentDictionary<string, Texture>();
            s_textureSamplers = new ConcurrentDictionary<TextureInput, TextureSampler>();

            s_models = new ConcurrentDictionary<string, Model>();

            s_fonts = new ConcurrentDictionary<string, Font>();

            s_collisionShapes = new ConcurrentDictionary<string, CollisionShape>();
        }

        static string GetPath(string a_path)
        {
            if (!Application.IsEditor)
            {   
                return ModControl.GetAssetPath(a_path);
            }

            return a_path;
        }

        public static void ClearAssets()
        {
            foreach (VertexShader vShader in s_vertexShaders.Values)
            {
                if (!vShader.IsDisposed)
                {
                    vShader.Dispose();
                }
            }
            s_vertexShaders.Clear();

            foreach (PixelShader pShader in s_pixelShaders.Values)
            {
                if (!pShader.IsDisposed)
                {
                    pShader.Dispose();
                }
            }
            s_pixelShaders.Clear();

            foreach (Material mat in s_materials.Values)
            {
                if (!mat.IsDisposed)
                {
                    mat.Dispose();
                }
            }
            s_materials.Clear();

            foreach (Texture texture in s_textures.Values)
            {
                if (!texture.IsDisposed)
                {
                    texture.Dispose();
                }
            }
            s_textures.Clear();
            foreach (TextureSampler sampler in s_textureSamplers.Values)
            {
                if (!sampler.IsDisposed)
                {
                    sampler.Dispose();
                }
            }
            s_textureSamplers.Clear();

            foreach (Model model in s_models.Values)
            {
                if (!model.IsDisposed)
                {
                    model.Dispose();
                }
            }
            s_models.Clear();

            foreach (Font font in s_fonts.Values)
            {
                if (!font.IsDisposed)
                {
                    font.Dispose();
                }
            }
            s_fonts.Clear();

            foreach (CollisionShape shape in s_collisionShapes.Values)
            {
                if (shape is IDisposable disp)
                {
                    if (disp is IDestroy dest)
                    {
                        if (!dest.IsDisposed)
                        {
                            dest.Dispose();
                        }
                    }
                    else
                    {
                        disp.Dispose();
                    }
                }
            }
            s_collisionShapes.Clear();
        }

        public static VertexShader LoadVertexShader(string a_path)
        {
            VertexShader oldShader = null;
            if (s_vertexShaders.ContainsKey(a_path))
            {
                oldShader = s_vertexShaders[a_path];
                if (!oldShader.IsDisposed)
                {
                    return oldShader;
                }
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }
            
            VertexShader shader = VertexShader.LoadVertexShader(filepath);
            if (shader == null)
            {
                Logger.IcarianError($"Error loading VertexShader: {a_path}, at {filepath}");

                return null;
            }

            if (oldShader == null)
            {
                s_vertexShaders.TryAdd(a_path, shader);
            }
            else
            {
                s_vertexShaders.TryUpdate(a_path, shader, oldShader);
            }

            return shader;
        }
        public static PixelShader LoadPixelShader(string a_path)
        {
            PixelShader oldShader = null;
            if (s_pixelShaders.ContainsKey(a_path))
            {
                oldShader = s_pixelShaders[a_path];
                if (!oldShader.IsDisposed)
                {
                    return oldShader;
                }
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }
        
            PixelShader shader = PixelShader.LoadPixelShader(filepath);
            if (shader == null)
            {
                Logger.IcarianError($"Error loading PixelShader: {a_path} at {filepath}");

                return null;
            }

            if (oldShader == null)
            {
                s_pixelShaders.TryAdd(a_path, shader);
            }   
            else
            {
                s_pixelShaders.TryUpdate(a_path, shader, oldShader);
            }

            return shader;
        }
        public static Font LoadFont(string a_path)
        {
            Font oldFont = null;
            if (s_fonts.ContainsKey(a_path))
            {
                oldFont = s_fonts[a_path];
                if (!oldFont.IsDisposed)
                {
                    return oldFont;
                }
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            Font font = Font.LoadFont(filepath);
            if (font == null)
            {
                Logger.IcarianError($"Error loading Font: {a_path} at {filepath}");

                return null;
            }

            if (oldFont == null)
            {
                s_fonts.TryAdd(a_path, font);
            }
            else
            {
                s_fonts.TryUpdate(a_path, font, oldFont);
            }

            return font;
        }

        public static Model LoadModel(string a_path)
        {
            Model oldModel = null;
            if (s_models.ContainsKey(a_path))
            {
                oldModel = s_models[a_path];
                if (!oldModel.IsDisposed)
                {
                    return oldModel;
                }
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }        

            Model model = Model.LoadModel(filepath);
            if (model == null)
            {
                Logger.IcarianError($"Error loading Model: {a_path} at {filepath}");

                return null;
            }

            if (oldModel == null)
            {
                s_models.TryAdd(a_path, model);
            }
            else
            {
                s_models.TryUpdate(a_path, model, oldModel);
            }

            return model;
        }

        public static Texture LoadTexture(string a_path)
        {
            Texture oldTexture = null;
            if (s_textures.ContainsKey(a_path))
            {
                oldTexture = s_textures[a_path];
                if (!oldTexture.IsDisposed)
                {
                    return oldTexture;
                }
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            Texture texture = Texture.LoadTexture(filepath);
            if (texture == null)
            {
                Logger.IcarianError($"Error loading Texture: {a_path} at {filepath}");

                return null;
            }

            if (oldTexture == null)
            {
                s_textures.TryAdd(a_path, texture);
            }
            else
            {
                s_textures.TryUpdate(a_path, texture, oldTexture);
            }

            return texture;
        }
        public static TextureSampler GetSampler(TextureInput a_input)
        {
            TextureSampler oldSampler = null;
            if (s_textureSamplers.ContainsKey(a_input))
            {
                oldSampler = s_textureSamplers[a_input];
                if (!oldSampler.IsDisposed)
                {
                    return oldSampler;
                }
            }

            Texture texture = LoadTexture(a_input.Path);
            if (texture == null)
            {
                Logger.IcarianError("Failed to load texture for sampler");
                
                return null;
            }

            TextureSampler sampler = TextureSampler.GeneretateTextureSampler(texture, a_input.FilterMode, a_input.AddressMode);
            if (sampler != null)
            {
                if (oldSampler == null)
                {
                    s_textureSamplers.TryAdd(a_input, sampler);
                }
                else
                {
                    s_textureSamplers.TryUpdate(a_input, sampler, oldSampler);
                }
            }

            return sampler;
        }

        public static Material GetMaterial(MaterialDef a_def)
        {
            if (a_def == null)
            {
                Logger.IcarianWarning("Null MaterialDef");

                return null;
            }

            Material oldMat = null;
            string str = $"[{a_def.VertexShaderPath}] [{a_def.PixelShaderPath}]";
            if (s_materials.ContainsKey(str))
            {
                oldMat = s_materials[str];
                if (!oldMat.IsDisposed)
                {
                    return oldMat;
                }
            }

            Material mat = Material.FromDef(a_def);
            if (mat != null)
            {
                if (oldMat == null)
                {
                    s_materials.TryAdd(str, mat);
                }
                else
                {
                    s_materials.TryUpdate(str, mat, oldMat);
                }
            }

            return mat;
        }

        public static CollisionShape GetCollisionShape(CollisionShapeDef a_def)
        {
            if (a_def == null)
            {
                Logger.IcarianWarning("Null CollisionShapeDef");

                return null;
            }

            CollisionShape oldShape = null;
            if (s_collisionShapes.ContainsKey(a_def.DefName))
            {
                oldShape = s_collisionShapes[a_def.DefName];
                IDestroy dest = oldShape as IDestroy;
                if (dest != null)
                {
                    if (!dest.IsDisposed)
                    {
                        return oldShape;
                    }
                }
                else
                {
                    return oldShape;
                }
            }

            CollisionShape shape = CollisionShape.FromDef(a_def);
            if (shape != null)
            {
                if (oldShape == null)
                {
                    s_collisionShapes.TryAdd(a_def.DefName, shape);
                }
                else
                {
                    s_collisionShapes.TryUpdate(a_def.DefName, shape, oldShape);
                }
            }

            return shape;
        }
    }
}