using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
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

    class GetMaterialThreadJob : IThreadJob
    {
        MaterialDef                       m_def;
        AssetLibrary.GetMaterialCallback m_callback;

        public GetMaterialThreadJob(MaterialDef a_def, AssetLibrary.GetMaterialCallback a_callback)
        {
            m_def = a_def;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Material material = AssetLibrary.GetMaterialInternal(m_def, out status);

            if (m_callback != null)
            {
                m_callback(material, status);
            }
        }
    }
    class LoadVertexShaderThreadJob : IThreadJob
    {
        string                                m_path;
        AssetLibrary.LoadVertexShaderCallback m_callback;

        public LoadVertexShaderThreadJob(string a_path, AssetLibrary.LoadVertexShaderCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            VertexShader shader = AssetLibrary.LoadVertexShaderInternal(m_path, out status);

            if (m_callback != null)
            {
                m_callback(shader, status);
            }
        }
    }
    class LoadPixelShaderThreadJob : IThreadJob
    {
        string                               m_path;
        AssetLibrary.LoadPixelShaderCallback m_callback;

        public LoadPixelShaderThreadJob(string a_path, AssetLibrary.LoadPixelShaderCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            PixelShader shader = AssetLibrary.LoadPixelShaderInternal(m_path, out status);

            if (m_callback != null)
            {
                m_callback(shader, status);
            }
        }
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
    class LoadSkinnedModelThreadJob : IThreadJob
    {
        string                         m_path;
        AssetLibrary.LoadModelCallback m_callback;

        public LoadSkinnedModelThreadJob(string a_path, AssetLibrary.LoadModelCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Model model = AssetLibrary.LoadSkinnedModelInternal(m_path, out status);

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
    class LoadSkeletonThreadJob : IThreadJob
    {
        string                            m_path;
        AssetLibrary.LoadSkeletonCallback m_callback;

        public LoadSkeletonThreadJob(string a_path, AssetLibrary.LoadSkeletonCallback a_callback)
        {
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Skeleton skeleton = AssetLibrary.LoadSkeletonInternal(m_path, out status);

            if (m_callback != null)
            {
                m_callback(skeleton, status);
            }
        }
    }

    interface IAssetContainer
    {
        LoadStatus Status
        {
            get;
            set;
        }
        EventWaitHandle WaitHandle
        {
            get;
        }
    }
    
    class MaterialContainer : IAssetContainer
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
        public Material Material
        {
            get;
            set;
        }

        public MaterialContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Material = null;
        }
    }
    class VertexShaderContainer : IAssetContainer
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
        public VertexShader Shader
        {
            get;
            set;
        }

        public VertexShaderContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Shader = null;
        }
    }
    class PixelShaderContainer : IAssetContainer
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
        public PixelShader Shader
        {
            get;
            set;
        }

        public PixelShaderContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Shader = null;
        }
    }
    class ModelContainer : IAssetContainer
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
    class TextureContainer : IAssetContainer
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
    class SkeletonContainer : IAssetContainer
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
        public Skeleton Skeleton
        {
            get;
            set;
        }

        public SkeletonContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Skeleton = null;
        }
    }

    public static class AssetLibrary
    {
        static ConcurrentDictionary<string, MaterialContainer>     s_materials;
        static ConcurrentDictionary<string, VertexShaderContainer> s_vertexShaders;
        static ConcurrentDictionary<string, PixelShaderContainer>  s_pixelShaders;
 
        static ConcurrentDictionary<string, TextureContainer>      s_textures;
        static ConcurrentDictionary<TextureInput, TextureSampler>  s_textureSamplers;
 
        static ConcurrentDictionary<string, ModelContainer>        s_models;
        static ConcurrentDictionary<string, ModelContainer>        s_skinnedModels;
 
        static ConcurrentDictionary<string, SkeletonContainer>     s_skeletons;
 
        static ConcurrentDictionary<string, Font>                  s_fonts;
 
        static ConcurrentDictionary<string, CollisionShape>        s_collisionShapes;

        public delegate void GetMaterialCallback(Material a_material, LoadStatus a_status);
        public delegate void LoadVertexShaderCallback(VertexShader a_shader, LoadStatus a_status);
        public delegate void LoadPixelShaderCallback(PixelShader a_shader, LoadStatus a_status);
        public delegate void LoadModelCallback(Model a_model, LoadStatus a_status);
        public delegate void LoadTextureCallback(Texture a_texture, LoadStatus a_status);
        public delegate void LoadSkeletonCallback(Skeleton a_skeleton, LoadStatus a_status);

        internal static void Init()
        {
            s_materials = new ConcurrentDictionary<string, MaterialContainer>();

            s_vertexShaders = new ConcurrentDictionary<string, VertexShaderContainer>();
            s_pixelShaders = new ConcurrentDictionary<string, PixelShaderContainer>();

            s_textures = new ConcurrentDictionary<string, TextureContainer>();
            s_textureSamplers = new ConcurrentDictionary<TextureInput, TextureSampler>();

            s_models = new ConcurrentDictionary<string, ModelContainer>();
            s_skinnedModels = new ConcurrentDictionary<string, ModelContainer>();

            s_skeletons = new ConcurrentDictionary<string, SkeletonContainer>();

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
            foreach (VertexShaderContainer vShader in s_vertexShaders.Values)
            {
                if (vShader.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (vShader.Status != LoadStatus.Loaded)
                {
                    vShader.WaitHandle.WaitOne();
                }

                if (vShader.Shader != null && !vShader.Shader.IsDisposed)
                {
                    vShader.Shader.Dispose();
                }
            }
            s_vertexShaders.Clear();

            foreach (PixelShaderContainer pShader in s_pixelShaders.Values)
            {
                if (pShader.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (pShader.Status != LoadStatus.Loaded)
                {
                    pShader.WaitHandle.WaitOne();
                }

                if (pShader.Shader != null && !pShader.Shader.IsDisposed)
                {
                    pShader.Shader.Dispose();
                }
            }
            s_pixelShaders.Clear();

            foreach (MaterialContainer mat in s_materials.Values)
            {
                if (mat.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (mat.Status != LoadStatus.Loaded)
                {
                    mat.WaitHandle.WaitOne();
                }

                if (mat.Material != null && !mat.Material.IsDisposed)
                {
                    mat.Material.Dispose();
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

            foreach (ModelContainer model in s_skinnedModels.Values)
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
            s_skinnedModels.Clear();

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

        static void ProcessContainer(IAssetContainer a_container) 
        {
            lock (a_container)
            {
                switch (a_container.Status)
                {
                case LoadStatus.Unloaded:
                {
                    a_container.Status = LoadStatus.Loading;

                    return;
                }
                case LoadStatus.Loading:
                {
                    break;
                }
                case LoadStatus.Loaded:
                case LoadStatus.Failed:
                {
                    return;
                }
                }
            }

            a_container.WaitHandle.WaitOne();
        }

        internal static VertexShader LoadVertexShaderInternal(string a_path, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            VertexShaderContainer container = null;
            if (s_vertexShaders.ContainsKey(a_path))
            {
                container = s_vertexShaders[a_path];
            }
            else
            {
                return null;
            }

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.Shader;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            VertexShader shader = VertexShader.LoadVertexShader(filepath);

            lock (container)
            {
                container.Shader = shader;

                if (shader != null)
                {
                    container.Status = LoadStatus.Loaded;
                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Status = LoadStatus.Failed;
                }
            }

            container.WaitHandle.Set();

            return shader;
        }
        public static VertexShader LoadVertexShader(string a_path)
        {
            if (s_vertexShaders.ContainsKey(a_path))
            {
                VertexShaderContainer c = s_vertexShaders[a_path];

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

                if (c.Shader != null)
                {
                    if (!c.Shader.IsDisposed)
                    {
                        return c.Shader;
                    }
                }
                else
                {
                    return null;
                }
            }

            s_vertexShaders.TryAdd(a_path, new VertexShaderContainer());

            return LoadVertexShaderInternal(a_path, out LoadStatus _);
        }
        public static void LoadVertexShaderAsync(string a_path, LoadVertexShaderCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_vertexShaders.TryAdd(a_path, new VertexShaderContainer());

            LoadVertexShaderThreadJob job = new LoadVertexShaderThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }
        internal static PixelShader LoadPixelShaderInternal(string a_path, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            PixelShaderContainer container = null;
            if (s_pixelShaders.ContainsKey(a_path))
            {
                container = s_pixelShaders[a_path];
            }
            else
            {
                return null;
            }

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.Shader;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            PixelShader shader = PixelShader.LoadPixelShader(filepath);

            lock (container)
            {
                container.Shader = shader;

                if (shader != null)
                {
                    container.Status = LoadStatus.Loaded;
                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Status = LoadStatus.Failed;
                }
            }

            container.WaitHandle.Set();

            return shader;
        }
        public static PixelShader LoadPixelShader(string a_path)
        {
            if (s_pixelShaders.ContainsKey(a_path))
            {
                PixelShaderContainer c = s_pixelShaders[a_path];

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

                if (c.Shader != null)
                {
                    if (!c.Shader.IsDisposed)
                    {
                        return c.Shader;
                    }
                }
                else
                {
                    return null;
                }
            }

            s_pixelShaders.TryAdd(a_path, new PixelShaderContainer());

            return LoadPixelShaderInternal(a_path, out LoadStatus _);
        }
        public static void LoadPixelShaderAsync(string a_path, LoadPixelShaderCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_pixelShaders.TryAdd(a_path, new PixelShaderContainer());

            LoadPixelShaderThreadJob job = new LoadPixelShaderThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
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

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

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

            return LoadModelInternal(a_path, out LoadStatus _);
        }
        public static void LoadModelAsync(string a_path, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_models.TryAdd(a_path, new ModelContainer());

            LoadModelThreadJob job = new LoadModelThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        internal static Model LoadSkinnedModelInternal(string a_path, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            ModelContainer container = null;
            if (s_skinnedModels.ContainsKey(a_path))
            {
                container = s_skinnedModels[a_path];
            }
            else
            {
                return null;
            }

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.Model;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            Model model = Model.LoadSkinnedModel(filepath);

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
        public static Model LoadSkinnedModel(string a_path)
        {
            if (s_skinnedModels.ContainsKey(a_path))
            {
                ModelContainer c = s_skinnedModels[a_path];

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

            s_skinnedModels.TryAdd(a_path, new ModelContainer());

            return LoadSkinnedModelInternal(a_path, out LoadStatus _);
        }
        public static void LoadSkinnedModelAsync(string a_path, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_skinnedModels.TryAdd(a_path, new ModelContainer());

            LoadSkinnedModelThreadJob job = new LoadSkinnedModelThreadJob(a_path, a_callback);

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

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

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

        internal static Skeleton LoadSkeletonInternal(string a_path, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            SkeletonContainer container = null;
            if (s_skeletons.ContainsKey(a_path))
            {
                container = s_skeletons[a_path];
            }
            else
            {
                return null;
            }

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.Skeleton;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            Skeleton skeleton = Skeleton.LoadSkeleton(filepath);

            lock (container)
            {
                if (skeleton != null)
                {
                    container.Skeleton = skeleton;
                    container.Status = LoadStatus.Loaded;

                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Skeleton = null;
                    container.Status = LoadStatus.Failed;
                }
            }

            container.WaitHandle.Set();

            return skeleton;
        }
        public static Skeleton LoadSkeleton(string a_path)
        {
            if (s_skeletons.ContainsKey(a_path))
            {
                SkeletonContainer c = s_skeletons[a_path];

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

                return c.Skeleton;
            }

            s_skeletons.TryAdd(a_path, new SkeletonContainer());

            return LoadSkeletonInternal(a_path, out LoadStatus _);
        }
        public static void LoadSkeletonAsync(string a_path, LoadSkeletonCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_skeletons.TryAdd(a_path, new SkeletonContainer());

            LoadSkeletonThreadJob job = new LoadSkeletonThreadJob(a_path, a_callback);

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

            TextureSampler sampler = TextureSampler.GenerateTextureSampler(texture, a_input.FilterMode, a_input.AddressMode);
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

        internal static Material GetMaterialInternal(MaterialDef a_def, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            MaterialContainer container = null;
            string str = $"{a_def.DefName}: [{a_def.VertexShaderPath}] [{a_def.PixelShaderPath}]";
            if (s_materials.ContainsKey(str))
            {
                container = s_materials[str];
            }
            else
            {
                return null;
            }
            
            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.Material;
            }

            Material mat = Material.FromDef(a_def);

            lock (container)
            {
                if (mat != null)
                {
                    container.Material = mat;
                    container.Status = LoadStatus.Loaded;

                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Material = null;
                    container.Status = LoadStatus.Failed;
                }
            }
            
            container.WaitHandle.Set();

            return mat;
        }
        public static Material GetMaterial(MaterialDef a_def)
        {
            if (a_def == null)
            {
                Logger.IcarianWarning("Null MaterialDef");

                return null;
            }

            string str = $"{a_def.DefName}: [{a_def.VertexShaderPath}] [{a_def.PixelShaderPath}]";
            if (s_materials.ContainsKey(str))
            {
                MaterialContainer c = s_materials[str];

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

                return c.Material;
            }

            s_materials.TryAdd(str, new MaterialContainer());

            return GetMaterialInternal(a_def, out LoadStatus _);
        }
        public static void GetMaterialAsync(MaterialDef a_def, GetMaterialCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (a_def == null)
            {
                Logger.IcarianWarning("Null MaterialDef");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            GetMaterialThreadJob job = new GetMaterialThreadJob(a_def, a_callback);

            ThreadPool.PushJob(job, a_priority);
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