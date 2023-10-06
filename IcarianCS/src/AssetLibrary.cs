using IcarianEngine.Audio;
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

    /// @cond INTERNAL
    class LoadAudioClipThreadJob : IThreadJob
    {
        ConcurrentDictionary<string, AudioClipContainer> m_clips;
        string                                           m_path;
        AssetLibrary.LoadAudioClipCallback               m_callback;

        public LoadAudioClipThreadJob(ConcurrentDictionary<string, AudioClipContainer> a_clips, string a_path, AssetLibrary.LoadAudioClipCallback a_callback)
        {
            m_clips = a_clips;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            AudioClip clip = AssetLibrary.LoadInternalData<AudioClip, AudioClipContainer>(m_path, m_clips, out status);

            if (m_callback != null)
            {
                m_callback(clip, status);
            }
        }
    }
    class GetMaterialThreadJob : IThreadJob
    {
        MaterialDef                      m_def;
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
        ConcurrentDictionary<string, VertexShaderContainer> m_shaders;
        string                                              m_path;
        AssetLibrary.LoadVertexShaderCallback               m_callback;

        public LoadVertexShaderThreadJob(ConcurrentDictionary<string, VertexShaderContainer> a_shaders, string a_path, AssetLibrary.LoadVertexShaderCallback a_callback)
        {
            m_shaders = a_shaders;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            VertexShader shader = AssetLibrary.LoadInternalData<VertexShader, VertexShaderContainer>(m_path, m_shaders, out status);

            if (m_callback != null)
            {
                m_callback(shader, status);
            }
        }
    }
    class LoadPixelShaderThreadJob : IThreadJob
    {
        ConcurrentDictionary<string, PixelShaderContainer> m_shaders;
        string                                             m_path;
        AssetLibrary.LoadPixelShaderCallback               m_callback;

        public LoadPixelShaderThreadJob(ConcurrentDictionary<string, PixelShaderContainer> a_shaders, string a_path, AssetLibrary.LoadPixelShaderCallback a_callback)
        {
            m_shaders = a_shaders;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            PixelShader shader = AssetLibrary.LoadInternalData<PixelShader, PixelShaderContainer>(m_path, m_shaders, out status);

            if (m_callback != null)
            {
                m_callback(shader, status);
            }
        }
    }
    class LoadModelThreadJob : IThreadJob
    {
        ConcurrentDictionary<string, ModelContainer> m_models;
        string                                       m_path;
        AssetLibrary.LoadModelCallback               m_callback;

        public LoadModelThreadJob(ConcurrentDictionary<string, ModelContainer> a_models, string a_path, AssetLibrary.LoadModelCallback a_callback)
        {
            m_models = a_models;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Model model = AssetLibrary.LoadInternalData<Model, ModelContainer>(m_path, m_models, out status);
 
            if (m_callback != null)
            {
                m_callback(model, status);
            }
        }
    }
    class LoadSkinnedModelThreadJob : IThreadJob
    {
        string                                       m_path;
        AssetLibrary.LoadModelCallback               m_callback;

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
        ConcurrentDictionary<string, TextureContainer> m_textures;
        string                                         m_path;
        AssetLibrary.LoadTextureCallback               m_callback;

        public LoadTextureThreadJob(ConcurrentDictionary<string, TextureContainer> a_textures, string a_path, AssetLibrary.LoadTextureCallback a_callback)
        {
            m_textures = a_textures;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Texture texture = AssetLibrary.LoadInternalData<Texture, TextureContainer>(m_path, m_textures, out status);

            if (m_callback != null)
            {
                m_callback(texture, status);
            }
        }
    }
    class LoadAnimationClipThreadJob : IThreadJob
    {
        ConcurrentDictionary<string, AnimationClipContainer> m_clips;
        string                                               m_path;
        AssetLibrary.LoadAnimationClipCallback               m_callback;

        public LoadAnimationClipThreadJob(ConcurrentDictionary<string, AnimationClipContainer> a_clips, string a_path, AssetLibrary.LoadAnimationClipCallback a_callback)
        {
            m_clips = a_clips;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            AnimationClip clip = AssetLibrary.LoadInternalData<AnimationClip, AnimationClipContainer>(m_path, m_clips, out status);

            if (m_callback != null)
            {
                m_callback(clip, status);
            }
        }
    }
    class LoadSkeletonThreadJob : IThreadJob
    {
        ConcurrentDictionary<string, SkeletonContainer> m_skeletons;
        string                                          m_path;
        AssetLibrary.LoadSkeletonCallback               m_callback;

        public LoadSkeletonThreadJob(ConcurrentDictionary<string, SkeletonContainer> a_skeletons, string a_path, AssetLibrary.LoadSkeletonCallback a_callback)
        {
            m_skeletons = a_skeletons;
            m_path = a_path;
            m_callback = a_callback;
        }

        public void Execute()
        {
            LoadStatus status;

            Skeleton skeleton = AssetLibrary.LoadInternalData<Skeleton, SkeletonContainer>(m_path, m_skeletons, out status);

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

        object Value
        {
            get;
            set;
        }

        object LoadValue(string a_input);
    }
    
    class AudioClipContainer : IAssetContainer
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
        public AudioClip Clip
        {
            get;
            set;
        }

        public object Value
        {
            get
            {
                return Clip;
            }
            set
            {
                Clip = (AudioClip)value;
            }
        }

        public AudioClipContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Clip = null;
        }

        public object LoadValue(string a_input)
        {
            return AudioClip.LoadAudioClip(a_input);
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

        public object Value
        {
            get
            {
                return Material;
            }
            set
            {
                Material = (Material)value;
            }
        }

        public MaterialContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Material = null;
        }

        public object LoadValue(string a_input) 
        { 
            return null; 
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

        public object Value
        {
            get
            {
                return Shader;
            }
            set
            {
                Shader = (VertexShader)value;
            }
        }

        public VertexShaderContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Shader = null;
        }

        public object LoadValue(string a_input)
        {
            return VertexShader.LoadVertexShader(a_input);
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

        public object Value
        {
            get
            {
                return Shader;
            }
            set
            {
                Shader = (PixelShader)value;
            }
        }

        public PixelShaderContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Shader = null;
        }

        public object LoadValue(string a_input)
        {
            return PixelShader.LoadPixelShader(a_input);
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

        public object Value
        {
            get
            {
                return Model;
            }
            set
            {
                Model = (Model)value;
            }
        }

        public ModelContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Model = null;
        }

        public object LoadValue(string a_input)
        {
            return Model.LoadModel(a_input);
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

        public object Value
        {
            get
            {
                return Texture;
            }
            set
            {
                Texture = (Texture)value;
            }
        }

        public TextureContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Texture = null;
        }

        public object LoadValue(string a_input)
        {
            return Texture.LoadTexture(a_input);
        }
    }
    class AnimationClipContainer : IAssetContainer
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
        public AnimationClip Clip
        {
            get;
            set;
        }

        public object Value
        {
            get
            {
                return Clip;
            }
            set
            {
                Clip = (AnimationClip)value;
            }
        }

        public AnimationClipContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Clip = null;
        }

        public object LoadValue(string a_input)
        {
            return AnimationClip.LoadAnimationClip(a_input);
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

        public object Value
        {
            get
            {
                return Skeleton;
            }
            set
            {
                Skeleton = (Skeleton)value;
            }
        }

        public SkeletonContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Skeleton = null;
        }

        public object LoadValue(string a_input)
        {
            return Skeleton.LoadSkeleton(a_input);
        }
    }
    /// @endcond

    public static class AssetLibrary
    {
        static ConcurrentDictionary<string, AudioClipContainer>     s_audioClips;

        static ConcurrentDictionary<string, MaterialContainer>      s_materials;
        static ConcurrentDictionary<string, VertexShaderContainer>  s_vertexShaders;
        static ConcurrentDictionary<string, PixelShaderContainer>   s_pixelShaders;
  
        static ConcurrentDictionary<string, TextureContainer>       s_textures;
        static ConcurrentDictionary<TextureInput, TextureSampler>   s_textureSamplers;
  
        static ConcurrentDictionary<string, ModelContainer>         s_models;
        static ConcurrentDictionary<string, ModelContainer>         s_skinnedModels;
 
        static ConcurrentDictionary<string, AnimationClipContainer> s_animationClips;

        static ConcurrentDictionary<string, SkeletonContainer>      s_skeletons;
  
        static ConcurrentDictionary<string, Font>                   s_fonts;
  
        static ConcurrentDictionary<string, CollisionShape>         s_collisionShapes;

        /// <summary>
        /// Delegate for loading an AudioClip async.
        /// </summary>
        public delegate void LoadAudioClipCallback(AudioClip a_clip, LoadStatus a_status);
        /// <summary>
        /// Delegate for getting a Material async.
        /// </summary>
        public delegate void GetMaterialCallback(Material a_material, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a VertexShader async.
        /// </summary>
        public delegate void LoadVertexShaderCallback(VertexShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a PixelShader async.
        /// </summary>
        public delegate void LoadPixelShaderCallback(PixelShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a Model async.
        /// </summary>
        public delegate void LoadModelCallback(Model a_model, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a Texture async.
        /// </summary>
        public delegate void LoadTextureCallback(Texture a_texture, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading an Skeleton async.
        /// </summary>
        public delegate void LoadSkeletonCallback(Skeleton a_skeleton, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading an AnimationClip async.
        /// </summary>
        public delegate void LoadAnimationClipCallback(AnimationClip a_clip, LoadStatus a_status);

        internal static void Init()
        {
            s_audioClips = new ConcurrentDictionary<string, AudioClipContainer>();

            s_materials = new ConcurrentDictionary<string, MaterialContainer>();

            s_vertexShaders = new ConcurrentDictionary<string, VertexShaderContainer>();
            s_pixelShaders = new ConcurrentDictionary<string, PixelShaderContainer>();

            s_textures = new ConcurrentDictionary<string, TextureContainer>();
            s_textureSamplers = new ConcurrentDictionary<TextureInput, TextureSampler>();

            s_models = new ConcurrentDictionary<string, ModelContainer>();
            s_skinnedModels = new ConcurrentDictionary<string, ModelContainer>();

            s_animationClips = new ConcurrentDictionary<string, AnimationClipContainer>();

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

        /// <summary>
        /// Clears all assets from the AssetLibrary.
        /// </summary>
        public static void ClearAssets()
        {
            foreach (AudioClipContainer clip in s_audioClips.Values)
            {
                if (clip.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (clip.Status != LoadStatus.Loaded)
                {
                    clip.WaitHandle.WaitOne();
                }

                if (clip.Clip != null && !clip.Clip.IsDisposed)
                {
                    clip.Clip.Dispose();
                }
            }

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

            s_skeletons.Clear();
            s_animationClips.Clear();
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
        static T LoadData<T, C>(string a_path, ConcurrentDictionary<string, C> a_data) where T : class where C : IAssetContainer
        {
            if (a_data.ContainsKey(a_path))
            {
                C container = a_data[a_path];

                switch (container.Status)
                {
                case LoadStatus.Loading:
                case LoadStatus.Unloaded:
                {
                    container.WaitHandle.WaitOne();
                    
                    break;
                }
                case LoadStatus.Failed:
                {
                    return null;
                }
                }

                if (container.Status == LoadStatus.Failed)
                {
                    return null;
                }

                if (container.Value != null)
                {
                    IDestroy dest = container.Value as IDestroy;
                    if (dest != null)
                    {
                        if (!dest.IsDisposed)
                        {
                            return container.Value as T;
                        }
                    }
                    else
                    {
                        return container.Value as T;
                    }
                }
            }

            a_data.TryAdd(a_path, (C)Activator.CreateInstance(typeof(C)));

            return LoadInternalData<T, C>(a_path, a_data, out LoadStatus _);
        }
        internal static T LoadInternalData<T, C>(string a_path, ConcurrentDictionary<string, C> a_data, out LoadStatus a_status) where T : class where C : IAssetContainer
        {
            a_status = LoadStatus.Failed;

            C container = default(C);
            if (a_data.ContainsKey(a_path))
            {
                container = a_data[a_path];
            }
            else
            {
                return null;
            }

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return (T)container.Value;
            }

            string filepath = GetPath(a_path);
            if (string.IsNullOrEmpty(filepath))
            {
                Logger.IcarianError($"Cannot find filepath: {a_path}");

                return null;
            }

            object obj = container.LoadValue(filepath);

            lock (container)
            {
                container.Value = obj;

                if (obj != null)
                {
                    container.Status = LoadStatus.Loaded;
                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Status = LoadStatus.Failed;
                }
            }

            return (T)obj;
        }

        /// <summary>
        /// Loads a AudioClip from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the AudioClip.</param>
        /// <returns>The AudioClip if it was loaded successfully, null otherwise.</returns>
        /// @see AudioClip.LoadAudioClip
        public static AudioClip LoadAudioClip(string a_path)
        {
            return LoadData<AudioClip, AudioClipContainer>(a_path, s_audioClips);
        }
        /// <summary>
        /// Loads a AudioClip from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the AudioClip.</param>
        /// <param name="a_callback">The callback to call when the AudioClip is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see AudioClip.LoadAudioClip
        public static void LoadAudioClipAsync(string a_path, LoadAudioClipCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_audioClips.TryAdd(a_path, new AudioClipContainer());

            LoadAudioClipThreadJob job = new LoadAudioClipThreadJob(s_audioClips, a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        /// <summary>
        /// Loads a VertexShader from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the VertexShader.</param>
        /// <returns>The VertexShader if it was loaded successfully, null otherwise.</returns>
        /// @see VertexShader.LoadVertexShader
        public static VertexShader LoadVertexShader(string a_path)
        {
            return LoadData<VertexShader, VertexShaderContainer>(a_path, s_vertexShaders);
        }
        /// <summary>
        /// Loads a VertexShader from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the VertexShader.</param>
        /// <param name="a_callback">The callback to call when the VertexShader is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see VertexShader.LoadVertexShader
        public static void LoadVertexShaderAsync(string a_path, LoadVertexShaderCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_vertexShaders.TryAdd(a_path, new VertexShaderContainer());

            LoadVertexShaderThreadJob job = new LoadVertexShaderThreadJob(s_vertexShaders, a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }
        /// <summary>
        /// Loads a PixelShader from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the PixelShader.</param>
        /// <returns>The PixelShader if it was loaded successfully, null otherwise.</returns>
        /// @see PixelShader.LoadPixelShader
        public static PixelShader LoadPixelShader(string a_path)
        {
            return LoadData<PixelShader, PixelShaderContainer>(a_path, s_pixelShaders);
        }
        /// <summary>
        /// Loads a PixelShader from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the PixelShader.</param>
        /// <param name="a_callback">The callback to call when the PixelShader is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see PixelShader.LoadPixelShader
        public static void LoadPixelShaderAsync(string a_path, LoadPixelShaderCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_pixelShaders.TryAdd(a_path, new PixelShaderContainer());

            LoadPixelShaderThreadJob job = new LoadPixelShaderThreadJob(s_pixelShaders, a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        /// <summary>
        /// Loads a Font from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Font.</param>
        /// <returns>The Font if it was loaded successfully, null otherwise.</returns>
        /// @warning Not thread safe.
        /// @see Font.LoadFont
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

        /// <summary>
        /// Loads a Model from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Model.</param>
        /// <returns>The Model if it was loaded successfully, null otherwise.</returns>
        /// @see Model.LoadModel
        public static Model LoadModel(string a_path)
        {
            return LoadData<Model, ModelContainer>(a_path, s_models);
        }
        /// <summary>
        /// Loads a Model from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Model.</param>
        /// <param name="a_callback">The callback to call when the Model is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see Model.LoadModel
        public static void LoadModelAsync(string a_path, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_models.TryAdd(a_path, new ModelContainer());

            LoadModelThreadJob job = new LoadModelThreadJob(s_models, a_path, a_callback);

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
        /// <summary>
        /// Loads a SkinnedModel from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the SkinnedModel.</param>
        /// <returns>The SkinnedModel if it was loaded successfully, null otherwise.</returns>
        /// @see Model.LoadSkinnedModel
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
        /// <summary>
        /// Loads a SkinnedModel from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the SkinnedModel.</param>
        /// <param name="a_callback">The callback to call when the SkinnedModel is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see Model.LoadSkinnedModel
        public static void LoadSkinnedModelAsync(string a_path, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_skinnedModels.TryAdd(a_path, new ModelContainer());

            LoadSkinnedModelThreadJob job = new LoadSkinnedModelThreadJob(a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        /// <summary>
        /// Loads a Texture from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Texture.</param>
        /// <returns>The Texture if it was loaded successfully, null otherwise.</returns>
        /// @see Texture.LoadTexture
        public static Texture LoadTexture(string a_path)
        {
            return LoadData<Texture, TextureContainer>(a_path, s_textures);
        }
        /// <summary>
        /// Loads a Texture from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Texture.</param>
        /// <param name="a_callback">The callback to call when the Texture is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see Texture.LoadTexture
        public static void LoadTextureAsync(string a_path, LoadTextureCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_textures.TryAdd(a_path, new TextureContainer());

            LoadTextureThreadJob job = new LoadTextureThreadJob(s_textures, a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        /// <summary>
        /// Loads an AnimationClip from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the AnimationClip.</param>
        /// <returns>The AnimationClip if it was loaded successfully, null otherwise.</returns>
        /// @see AnimationClip.LoadAnimationClip
        public static AnimationClip LoadAnimationClip(string a_path)
        {
            return LoadData<AnimationClip, AnimationClipContainer>(a_path, s_animationClips);
        }
        /// <summary>
        /// Loads an AnimationClip from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the AnimationClip.</param>
        /// <param name="a_callback">The callback to call when the AnimationClip is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see AnimationClip.LoadAnimationClip
        public static void LoadAnimationClipAsync(string a_path, LoadAnimationClipCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_animationClips.TryAdd(a_path, new AnimationClipContainer());

            LoadAnimationClipThreadJob job = new LoadAnimationClipThreadJob(s_animationClips, a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        /// <summary>
        /// Loads a Skeleton from the given path in a mod.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Skeleton.</param>
        /// <returns>The Skeleton if it was loaded successfully, null otherwise.</returns>
        /// @see Skeleton.LoadSkeleton
        public static Skeleton LoadSkeleton(string a_path)
        {
            return LoadData<Skeleton, SkeletonContainer>(a_path, s_skeletons);
        }
        /// <summary>
        /// Loads a Skeleton from the given path in a mod asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Skeleton.</param>
        /// <param name="a_callback">The callback to call when the Skeleton is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see Skeleton.LoadSkeleton
        public static void LoadSkeletonAsync(string a_path, LoadSkeletonCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            s_skeletons.TryAdd(a_path, new SkeletonContainer());

            LoadSkeletonThreadJob job = new LoadSkeletonThreadJob(s_skeletons, a_path, a_callback);

            ThreadPool.PushJob(job, a_priority);
        }

        /// <summary>
        /// Gets a TextureSampler from the given TextureInput.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_input">The TextureInput to get the TextureSampler from.</param>
        /// <returns>The TextureSampler if it was loaded successfully, null otherwise.</returns>
        /// @warning Not thread safe.
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
        /// <summary>
        /// Gets a Material from the given MaterialDef.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The MaterialDef to get the Material from.</param>
        /// <returns>The Material if it was loaded successfully, null otherwise.</returns>
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
        /// <summary>
        /// Gets a Material from the given MaterialDef asynchronously.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The MaterialDef to get the Material from.</param>
        /// <param name="a_callback">The callback to call when the Material is loaded.</param>
        /// <param name="a_priority">The priority of the job.</param>
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

        /// <summary>
        /// Gets a CollisionShape from the given CollisionShapeDef.
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The CollisionShapeDef to get the CollisionShape from.</param>
        /// <returns>The CollisionShape if it was loaded successfully, null otherwise.</returns>
        /// @warning Not thread safe.
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