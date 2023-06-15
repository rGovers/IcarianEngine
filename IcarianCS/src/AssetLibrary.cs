using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.UI;
using System;
using System.Collections.Concurrent;
using System.Threading;

namespace IcarianEngine
{
    public enum LoadStatus
    {
        Unloaded,
        Loading,
        Loaded,
        Failed
    }

    class LoadModelThreadJob : IThreadJob
    {
        string                         m_path;
        AssetLibrary.LoadModelCallback m_callback;

        public LoadModelThreadJob(string a_path, AssetLibrary.LoadModelCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Model model = AssetLibrary.LoadModelInternal(m_path, out status);
 
            if (m_callback != null)
            {
                m_callback(model, status);
            }
        }
    }
    class LoadTextureThreadJob : IThreadJob
    {
        string                           m_path;
        AssetLibrary.LoadTextureCallback m_callback;

        public LoadTextureThreadJob(string a_path, AssetLibrary.LoadTextureCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Texture texture = AssetLibrary.LoadTextureInternal(m_path, out status);

            if (m_callback != null)
            {
                m_callback(texture, status);
            }
        }
    }

    class ModelContainer
    {
        public LoadStatus Status
        {
            get;
            set;
        }
        public EventWaitHandle WaitHandle
        {
            get;
            set;
        }
        public Model Model
        {
            get;
            set;
        }

        public ModelContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Model = null;
        }
    }
    class TextureContainer
    {
        public LoadStatus Status
        {
            get;
            set;
        }
        public EventWaitHandle WaitHandle
        {
            get;
            set;
        }
        public Texture Texture
        {
            get;
            set;
        }

        public TextureContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Texture = null;
        }
    }

    public static class AssetLibrary
    {
        static ConcurrentDictionary<string, Material>             s_materials;
        static ConcurrentDictionary<string, VertexShader>         s_vertexShaders;
        static ConcurrentDictionary<string, PixelShader>          s_pixelShaders;

        static ConcurrentDictionary<string, TextureContainer>     s_textures;
        static ConcurrentDictionary<TextureInput, TextureSampler> s_textureSamplers;

        static ConcurrentDictionary<string, ModelContainer>       s_models;
     
        static ConcurrentDictionary<string, Font>                 s_fonts;

        static ConcurrentDictionary<string, CollisionShape>       s_collisionShapes;

        public delegate void LoadModelCallback(Model a_model, LoadStatus a_status);
        public delegate void LoadTextureCallback(Texture a_texture, LoadStatus a_status);

        internal static void Init()
        {
            s_materials = new ConcurrentDictionary<string, Material>();

            s_vertexShaders = new ConcurrentDictionary<string, VertexShader>();
            s_pixelShaders = new ConcurrentDictionary<string, PixelShader>();

            s_textures = new ConcurrentDictionary<string, TextureContainer>();
            s_textureSamplers = new ConcurrentDictionary<TextureInput, TextureSampler>();

            s_models = new ConcurrentDictionary<string, ModelContainer>();

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

            foreach (TextureContainer texture in s_textures.Values)
            {
                if (texture.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (texture.Status != LoadStatus.Loaded)
                {
                    texture.WaitHandle.WaitOne();
                }

                if (texture.Texture != null && !texture.Texture.IsDisposed)
                {
                    texture.Texture.Dispose();
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

            foreach (ModelContainer model in s_models.Values)
            {
                if (model.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (model.Status != LoadStatus.Loaded)
                {
                    model.WaitHandle.WaitOne();
                }

                if (model.Model != null && !model.Model.IsDisposed)
                {
                    model.Model.Dispose();
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

        internal static Model LoadModelInternal(string a_path, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            ModelContainer container = null;
            if (s_models.ContainsKey(a_path))
            {
                container = s_models[a_path];
            }
            else
            {
                return null;
            }

            bool alreadyLoading = false;

            lock (container)
            {
                switch (container.Status)
                {
                case LoadStatus.Unloaded:
                {
                    container.Status = LoadStatus.Loading;

                    break;
                }
                case LoadStatus.Loading:
                {
                    alreadyLoading = true;

                    break;
                }
                case LoadStatus.Loaded:
                {
                    a_status = LoadStatus.Loaded;

                    return container.Model;
                }
                case LoadStatus.Failed:
                {
                    return null;
                }
                }
            }

            if (alreadyLoading)
            {
                container.WaitHandle.WaitOne();
                a_status = LoadStatus.Loaded;

                return container.Model;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }        

            Model model = Model.LoadModel(filepath);

            lock (container)
            {
                if (model != null)
                {
                    container.Model = model;
                    container.Status = LoadStatus.Loaded;

                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Model = null;
                    container.Status = LoadStatus.Failed;
                }
            }

            container.WaitHandle.Set();

            return model;
        }
        public static Model LoadModel(string a_path)
        {
            if (s_models.ContainsKey(a_path))
            {
                ModelContainer c = s_models[a_path];

                switch (c.Status)
                {
                case LoadStatus.Loading:
                case LoadStatus.Unloaded:
                {
                    c.WaitHandle.WaitOne();
                    
                    break;
                }
                case LoadStatus.Failed:
                {
                    return null;
                }
                }

                if (c.Model != null)
                {
                    if (!c.Model.IsDisposed)
                    {
                        return c.Model;
                    }
                }
                else
                {
                    return c.Model;
                }
            }

            s_models.TryAdd(a_path, new ModelContainer());

            Model model = LoadModelInternal(a_path, out LoadStatus _);

            return model;
        }
        public static void LoadModelAsync(string a_path, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_models.TryAdd(a_path, new ModelContainer());

            LoadModelThreadJob job = new LoadModelThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        internal static Texture LoadTextureInternal(string a_path, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            TextureContainer container = null;
            if (s_textures.ContainsKey(a_path))
            {
                container = s_textures[a_path];
            }
            else
            {
                return null;
            }

            bool alreadyLoading = false;

            lock (container)
            {
                switch (container.Status)
                {
                case LoadStatus.Unloaded:
                {
                    container.Status = LoadStatus.Loading;

                    break;
                }
                case LoadStatus.Loading:
                {
                    alreadyLoading = true;

                    break;
                }
                case LoadStatus.Loaded:
                {
                    a_status = LoadStatus.Loaded;

                    return container.Texture;
                }
                case LoadStatus.Failed:
                {
                    return null;
                }
                }
            }

            if (alreadyLoading)
            {
                container.WaitHandle.WaitOne();
                a_status = LoadStatus.Loaded;

                return container.Texture;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            Texture texture = Texture.LoadTexture(filepath);

            lock (container)
            {
                if (texture != null)
                {
                    container.Texture = texture;
                    container.Status = LoadStatus.Loaded;

                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Texture = null;
                    container.Status = LoadStatus.Failed;
                }
            }

            container.WaitHandle.Set();

            return texture;
        }
        public static Texture LoadTexture(string a_path)
        {
            if (s_textures.ContainsKey(a_path))
            {
                TextureContainer c = s_textures[a_path];

                switch (c.Status)
                {
                case LoadStatus.Loading:
                case LoadStatus.Unloaded:
                {
                    c.WaitHandle.WaitOne();
                    
                    break;
                }
                case LoadStatus.Failed:
                {
                    return null;
                }
                }

                if (c.Texture != null)
                {
                    if (!c.Texture.IsDisposed)
                    {
                        return c.Texture;
                    }
                }
                else
                {
                    return null;
                }
            }

            s_textures.TryAdd(a_path, new TextureContainer());

            Texture texture = LoadTextureInternal(a_path, out LoadStatus _);

            return texture;
        }
        public static void LoadTextureAsync(string a_path, LoadTextureCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_textures.TryAdd(a_path, new TextureContainer());

            LoadTextureThreadJob job = new LoadTextureThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
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