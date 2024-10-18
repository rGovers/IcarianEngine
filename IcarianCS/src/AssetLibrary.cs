// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Audio;
using IcarianEngine.Definitions;
using IcarianEngine.Mod;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
using IcarianEngine.Rendering.UI;
#ifdef ENABLE_EXPERIMENTAL
using IcarianEngine.Rendering.Video;
#endif
using System;
using System.Collections.Concurrent;

namespace IcarianEngine
{
    /// <summary>
    /// The status of a load operation.
    /// </summary>
    public enum LoadStatus
    {
        Unloaded,
        Loading,
        Loaded,
        Failed
    }

    public static class AssetLibrary
    {
        static ConcurrentDictionary<string, AudioClipContainer>      s_audioClips;
#ifdef ENABLE_EXPERIMENTAL
        static ConcurrentDictionary<string, VideoClipContainer>      s_videoClips;
 #endif

        static ConcurrentDictionary<string, MaterialContainer>       s_materials;
        static ConcurrentDictionary<string, VertexShaderContainer>   s_vertexShaders;
        static ConcurrentDictionary<string, PixelShaderContainer>    s_pixelShaders;
   
        static ConcurrentDictionary<string, TextureContainer>        s_textures;
        static ConcurrentDictionary<string, TextureSamplerContainer> s_textureSamplers;
   
        static ConcurrentDictionary<string, ModelContainer>          s_models;
        static ConcurrentDictionary<string, ModelContainer>          s_skinnedModels;
  
        static ConcurrentDictionary<string, AnimationClipContainer>  s_animationClips;
 
        static ConcurrentDictionary<string, SkeletonContainer>       s_skeletons;
   
        static ConcurrentDictionary<string, FontContainer>           s_fonts;
  
        static ConcurrentDictionary<string, CollisionShapeContainer> s_collisionShapes;

        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Audio.AudioClip" /> async
        /// </summary>
        public delegate void LoadAudioClipCallback(AudioClip a_clip, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.VertexShader" /> async
        /// </summary>
        public delegate void LoadVertexShaderCallback(VertexShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.PixelShader" /> async
        /// </summary>
        public delegate void LoadPixelShaderCallback(PixelShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Font" /> async
        /// </summary>
        public delegate void LoadFontCallback(Font a_font, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Model" /> async
        /// </summary>
        public delegate void LoadModelCallback(Model a_model, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Texture" /> async
        /// </summary>
        public delegate void LoadTextureCallback(Texture a_texture, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Animation.Skeleton" /> async
        /// </summary>
        public delegate void LoadSkeletonCallback(Skeleton a_skeleton, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /> async
        /// </summary>
        public delegate void LoadAnimationClipCallback(AnimationClip a_clip, LoadStatus a_status);

        /// <summary>
        /// Delegate for getting a <see cref="IcarianEngine.Rendering.TextureSampler" /> async
        /// </summary>
        public delegate void GetTextureSamplerCallback(TextureSampler a_sampler, LoadStatus a_status);
        /// <summary>
        /// Delegate for getting a <see cref="IcarianEngine.Rendering.Material" /> async
        /// </summary>
        public delegate void GetMaterialCallback(Material a_material, LoadStatus a_status);
        /// <summary>
        /// Delegate for getting a <see cref="IcarianEngine.Physics.Shapes.CollisionShape" /> async
        /// </summary>
        public delegate void GetCollisionShapeCallback(CollisionShape a_shape, LoadStatus a_status);

        internal static void Init()
        {
            s_audioClips = new ConcurrentDictionary<string, AudioClipContainer>();
#ifdef ENABLE_EXPERIMENTAL
            s_videoClips = new ConcurrentDictionary<string, VideoClipContainer>();
#endif

            s_materials = new ConcurrentDictionary<string, MaterialContainer>();

            s_vertexShaders = new ConcurrentDictionary<string, VertexShaderContainer>();
            s_pixelShaders = new ConcurrentDictionary<string, PixelShaderContainer>();

            s_textures = new ConcurrentDictionary<string, TextureContainer>();
            s_textureSamplers = new ConcurrentDictionary<string, TextureSamplerContainer>();

            s_models = new ConcurrentDictionary<string, ModelContainer>();
            s_skinnedModels = new ConcurrentDictionary<string, ModelContainer>();

            s_animationClips = new ConcurrentDictionary<string, AnimationClipContainer>();

            s_skeletons = new ConcurrentDictionary<string, SkeletonContainer>();

            s_fonts = new ConcurrentDictionary<string, FontContainer>();

            s_collisionShapes = new ConcurrentDictionary<string, CollisionShapeContainer>();
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
        /// Clears all assets from the AssetLibrary
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
#ifdef ENABLE_EXPERIMENTAL
            foreach (VideoClipContainer clip in s_videoClips.Values)
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
#endif

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

            foreach (TextureSamplerContainer sampler in s_textureSamplers.Values)
            {
                if (sampler.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (sampler.Status != LoadStatus.Loaded)
                {
                    sampler.WaitHandle.WaitOne();
                }

                if (sampler.Sampler != null && !sampler.Sampler.IsDisposed)
                {
                    sampler.Sampler.Dispose();
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

            foreach (FontContainer font in s_fonts.Values)
            {
                if (font.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (font.Status != LoadStatus.Loaded)
                {
                    font.WaitHandle.WaitOne();
                }

                if (font.Font != null && !font.Font.IsDisposed)
                {
                    font.Font.Dispose();
                }
            }
            s_fonts.Clear();

            foreach (CollisionShapeContainer shape in s_collisionShapes.Values)
            {
                if (shape.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (shape.Status != LoadStatus.Loaded)
                {
                    shape.WaitHandle.WaitOne();
                }

                if (shape.CollisionShape is IDestroy dest)
                {
                    if (!dest.IsDisposed)
                    {
                        dest.Dispose();
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

        static T WaitContainer<T>(IAssetContainer a_container) where T : class
        {
            switch (a_container.Status)
            {
            case LoadStatus.Loading:
            case LoadStatus.Unloaded:
            {
                a_container.WaitHandle.WaitOne();
                
                break;
            }
            case LoadStatus.Failed:
            {
                return null;
            }
            }

            if (a_container.Status == LoadStatus.Failed)
            {
                return null;
            }

            return a_container.Value as T;
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
        internal static T LoadInternalData<T, C>(string a_path, ConcurrentDictionary<string, C> a_data, out LoadStatus a_status) 
            where T : class 
            where C : IAssetContainer
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
        /// Loads a <see cref="IcarianEngine.Audio.AudioClip" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Audio.AudioClip" /></param>
        /// <returns>The <see cref="IcarianEngine.Audio.AudioClip" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Audio.AudioClip.LoadAudioClip
        public static AudioClip LoadAudioClip(string a_path)
        {
            return LoadData<AudioClip, AudioClipContainer>(a_path, s_audioClips);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Audio.AudioClip" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Audio.AudioClip" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Audio.AudioClip" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Audio.AudioClip.LoadAudioClip
        public static void LoadAudioClipAsync(string a_path, LoadAudioClipCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Audio Clip Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_audioClips.TryAdd(a_path, new AudioClipContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                AudioClip clip = LoadInternalData<AudioClip, AudioClipContainer>(a_path, s_audioClips, out status);

                if (a_callback != null)
                {
                    a_callback(clip, status);
                }
            }, a_priority);
        }

#ifdef ENABLE_EXPERIMENTAL
        public static VideoClip LoadVideoClip(string a_path)
        {
            return LoadData<VideoClip, VideoClipContainer>(a_path, s_videoClips);
        }
#endif

        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.VertexShader" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.VertexShader" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.VertexShader" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Rendering.VertexShader.LoadVertexShader
        public static VertexShader LoadVertexShader(string a_path)
        {
            return LoadData<VertexShader, VertexShaderContainer>(a_path, s_vertexShaders);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.VertexShader" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.VertexShader" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.VertexShader" /> is loaded</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see IcarianEngine.Rendering.VertexShader.LoadVertexShader
        public static void LoadVertexShaderAsync(string a_path, LoadVertexShaderCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Vertex Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_vertexShaders.TryAdd(a_path, new VertexShaderContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                VertexShader shader = LoadInternalData<VertexShader, VertexShaderContainer>(a_path, s_vertexShaders, out status);

                if (a_callback != null)
                {
                    a_callback(shader, status);
                }
            }, a_priority);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.PixelShader" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.PixelShader" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.PixelShader" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Rendering.PixelShader.LoadPixelShader
        public static PixelShader LoadPixelShader(string a_path)
        {
            return LoadData<PixelShader, PixelShaderContainer>(a_path, s_pixelShaders);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.PixelShader" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.PixelShader" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.PixelShader" /> is loaded</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see IcarianEngine.Rendering.PixelShader.LoadPixelShader
        public static void LoadPixelShaderAsync(string a_path, LoadPixelShaderCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Pixel Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_pixelShaders.TryAdd(a_path, new PixelShaderContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                PixelShader shader = LoadInternalData<PixelShader, PixelShaderContainer>(a_path, s_pixelShaders, out status);

                if (a_callback != null)
                {
                    a_callback(shader, status);
                }
            }, a_priority);
        }

        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Font" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Font" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Font" /> if it was loaded successfully, null otherwise</returns>
        /// @see IcarianEngine.Rendering.Font.LoadFont
        public static Font LoadFont(string a_path)
        {
            return LoadData<Font, FontContainer>(a_path, s_fonts);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Font" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchornously
        /// </summary>
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Font" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.Font" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Rendering.Font.LoadFont
        public static void LoadFontAsync(string a_path, LoadFontCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Font Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_fonts.TryAdd(a_path, new FontContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                Font font = LoadInternalData<Font, FontContainer>(a_path, s_fonts, out status);

                if (a_callback != null)
                {
                    a_callback(font, status);
                }
            }, a_priority);
        }

        internal static Model LoadModelInternal(string a_path, byte a_index, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            string str = $"[{a_index}] {a_path}";

            ModelContainer container = null;
            if (s_models.ContainsKey(str))
            {
                container = s_models[str];
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

            Model model = Model.LoadModel(filepath, a_index);

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
        /// Loads a <see cref="IcarianEngine.Rendering.Model" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Model" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Model" /> if it was loaded successfully, null otherwise</returns>
        /// @see IcarianEngine.Rendering.Model.LoadModel
        public static Model LoadModel(string a_path, byte a_index = byte.MaxValue)
        {
            string str = $"[{a_index}] {a_path}";

            if (s_models.ContainsKey(str))
            {
                ModelContainer c = s_models[str];

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

            s_models.TryAdd(str, new ModelContainer());

            return LoadModelInternal(a_path, a_index, out LoadStatus _);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Model" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Model" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.Model" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Rendering.Model.LoadModel
        public static void LoadModelAsync(string a_path, byte a_index, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Model Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            string str = $"[{a_index}] {a_path}";

            s_models.TryAdd(str, new ModelContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                Model model = LoadModelInternal(a_path, a_index, out status);
    
                if (a_callback != null)
                {
                    a_callback(model, status);
                }
            }, a_priority);
        }

        internal static Model LoadSkinnedModelInternal(string a_path, byte a_index, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            string str = $"[{a_index}] {a_path}";

            ModelContainer container = null;
            if (s_skinnedModels.ContainsKey(str))
            {
                container = s_skinnedModels[str];
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

            Model model = Model.LoadSkinnedModel(filepath, a_index);

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
        /// Loads a Skinned <see cref="IcarianEngine.Rendering.Model" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Skinned <see cref="IcarianEngine.Rendering.Model" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Model" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Rendering.Model.LoadSkinnedModel
        public static Model LoadSkinnedModel(string a_path, byte a_index = byte.MaxValue)
        {
            string str = $"[{a_index}] {a_path}";

            if (s_skinnedModels.ContainsKey(str))
            {
                ModelContainer c = s_skinnedModels[str];

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

            s_skinnedModels.TryAdd(str, new ModelContainer());

            return LoadSkinnedModelInternal(a_path, a_index, out LoadStatus _);
        }
        /// <summary>
        /// Loads a Skinned <see cref="IcarianEngine.Rendering.Model" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the Skinned <see cref="IcarianEngine.Rendering.Model" /></param>
        /// <param name="a_callback">The callback to call when the Skinned <see cref="IcarianEngine.Rendering.Model" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Rendering.Model.LoadSkinnedModel
        public static void LoadSkinnedModelAsync(string a_path, byte a_index, LoadModelCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Skinned Model Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            string str = $"[{a_index}] {a_path}";

            s_skinnedModels.TryAdd(str, new ModelContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                Model model = LoadSkinnedModelInternal(a_path, a_index, out status);

                if (a_callback != null)
                {
                    a_callback(model, status);
                }
            }, a_priority);
        }

        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Texture" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Texture" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Texture" /> if it was loaded successfully, null otherwise</returns>
        /// @see IcarianEngine.Rendering.Texture.LoadTexture
        public static Texture LoadTexture(string a_path)
        {
            return LoadData<Texture, TextureContainer>(a_path, s_textures);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Texture" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Texture" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.Texture" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Rendering.Texture.LoadTexture
        public static void LoadTextureAsync(string a_path, LoadTextureCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Texture Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_textures.TryAdd(a_path, new TextureContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                Texture texture = LoadInternalData<Texture, TextureContainer>(a_path, s_textures, out status);

                if (a_callback != null)
                {
                    a_callback(texture, status);
                }
            }, a_priority);
        }

        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Rendering.Animation.AnimationClip.LoadAnimationClip
        public static AnimationClip LoadAnimationClip(string a_path)
        {
            return LoadData<AnimationClip, AnimationClipContainer>(a_path, s_animationClips);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.Animation.AnimationClip" /> is loaded</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see IcarianEngine.Rendering.Animation.AnimationClip.LoadAnimationClip
        public static void LoadAnimationClipAsync(string a_path, LoadAnimationClipCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Animation Clip Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_animationClips.TryAdd(a_path, new AnimationClipContainer());

            ThreadPool.PushJob(() => 
            {
                LoadStatus status;

                AnimationClip clip = LoadInternalData<AnimationClip, AnimationClipContainer>(a_path, s_animationClips, out status);

                if (a_callback != null)
                {
                    a_callback(clip, status);
                }
            }, a_priority);
        }

        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Animation.Skeleton" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Animation.Skeleton" /></param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Animation.Skeleton" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Rendering.Animation.Skeleton.LoadSkeleton
        public static Skeleton LoadSkeleton(string a_path)
        {
            return LoadData<Skeleton, SkeletonContainer>(a_path, s_skeletons);
        }
        /// <summary>
        /// Loads a <see cref="IcarianEngine.Rendering.Animation.Skeleton" /> from the given path in a <see cref="IcarianEngine.Mod.IcarianAssembly" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_path">The path to the <see cref="IcarianEngine.Rendering.Animation.Skeleton" /></param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.Animation.Skeleton" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Rendering.Animation.Skeleton.LoadSkeleton
        public static void LoadSkeletonAsync(string a_path, LoadSkeletonCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (string.IsNullOrWhiteSpace(a_path))
            {
                Logger.IcarianWarning("Null Skeleton Path");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_skeletons.TryAdd(a_path, new SkeletonContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                Skeleton skeleton = LoadInternalData<Skeleton, SkeletonContainer>(a_path, s_skeletons, out status);

                if (a_callback != null)
                {
                    a_callback(skeleton, status);
                }
            }, a_priority);
        }

        internal static TextureSampler GetTextureSamplerInternal(TextureInput a_input, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            TextureSamplerContainer container = null;
            string str = $"{a_input.Path}: {a_input.AddressMode},{a_input.FilterMode}";
            if (s_textureSamplers.ContainsKey(str))
            {
                container = s_textureSamplers[str];
            }
            else
            {
                return null;
            }

            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.Sampler;
            }

            Texture tex = LoadTexture(a_input.Path);
            if (tex == null)
            {
                a_status = LoadStatus.Failed;

                return null;
            }

            TextureSampler sampler = TextureSampler.GenerateTextureSampler(tex, a_input.FilterMode, a_input.AddressMode);
            lock (container)
            {
                if (sampler != null)
                {
                    container.Sampler = sampler;
                    container.Status = LoadStatus.Loaded;

                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.Sampler = null;
                    container.Status = LoadStatus.Failed;
                }
            }

            container.WaitHandle.Set();

            return sampler;
        }
        /// <summary>
        /// Gets a <see cref="IcarianEngine.Rendering.TextureSampler" /> from the given <see cref="IcarianEngine.Definitions.TextureInput" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_input">The <see cref="IcarianEngine.Definitions.TextureInput" /> to get the <see cref="IcarianEngine.Rendering.TextureSampler" /> from</param>
        /// <returns>The <see cref="IcarianEngine.Rendering.TextureSampler" /> if it was loaded successfully, null otherwise</returns>
        public static TextureSampler GetSampler(TextureInput a_input)
        {
            string str = $"{a_input.Path}: {a_input.AddressMode},{a_input.FilterMode}";

            if (s_textureSamplers.ContainsKey(str))
            {
                return WaitContainer<TextureSampler>(s_textureSamplers[str]);
            }

            s_textureSamplers.TryAdd(str, new TextureSamplerContainer());

            return GetTextureSamplerInternal(a_input, out LoadStatus _);
        }
        /// <summary>
        /// Gets a <see cref="IcarianEngine.Rendering.TextureSampler" /> from the given <see cref="IcarianEngine.Definitions.TextureInput" /> asynchronously
        /// </summary>
        /// <param name="a_input">The <see cref="IcarianEngine.Definitions.TextureInput" /> to get the <see cref="IcarianEngine.Rendering.TextureSampler" /> from</param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.TextureSampler" /> is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        public static void GetTextureAsync(TextureInput a_input, GetTextureSamplerCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            string str = $"{a_input.Path}: {a_input.AddressMode},{a_input.FilterMode}";

            s_textureSamplers.TryAdd(str, new TextureSamplerContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                TextureSampler sampler = GetTextureSamplerInternal(a_input, out status);

                if (a_callback != null)
                {
                    a_callback(sampler, status);
                }
            }, a_priority);
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
        /// Gets a <see cref="IcarianEngine.Rendering.Material" /> from the given <see cref="IcarianEngine.Definitions.MaterialDef" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.MaterialDef" /> to get the <see cref="IcarianEngine.Rendering.Material" /> from</param>
        /// <returns>The <see cref="IcarianEngine.Rendering.Material" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Rendering.Material.FromDef
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
                return WaitContainer<Material>(s_materials[str]);
            }

            s_materials.TryAdd(str, new MaterialContainer());

            return GetMaterialInternal(a_def, out LoadStatus _);
        }
        /// <summary>
        /// Gets a <see cref="IcarianEngine.Rendering.Material" /> from the given <see cref="IcarianEngine.Definitions.MaterialDef" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.MaterialDef" /> to get the <see cref="IcarianEngine.Rendering.Material" /> from</param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Rendering.Material" /> is loaded</param>
        /// <param name="a_priority">The priority of the job.</param>
        /// @see IcarianEngine.Rendering.Material.FromDef
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

            string str = $"{a_def.DefName}: [{a_def.VertexShaderPath}] [{a_def.PixelShaderPath}]";
            s_materials.TryAdd(str, new MaterialContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                Material material = GetMaterialInternal(a_def, out status);

                if (a_callback != null)
                {
                    a_callback(material, status);
                }
            }, a_priority);
        }

        internal static CollisionShape GetCollisionShapeInternal(CollisionShapeDef a_def, out LoadStatus a_status)
        {
            a_status = LoadStatus.Failed;

            CollisionShapeContainer container = null;
            if (s_collisionShapes.ContainsKey(a_def.DefName))
            {
                container = s_collisionShapes[a_def.DefName];
            }
            else
            {
                return null;
            }
            
            ProcessContainer(container);

            if (container.Status != LoadStatus.Loading)
            {
                a_status = container.Status;

                return container.CollisionShape;
            }

            CollisionShape shape = CollisionShape.FromDef(a_def);

            lock (container)
            {
                if (shape != null)
                {
                    container.CollisionShape = shape;
                    container.Status = LoadStatus.Loaded;

                    a_status = LoadStatus.Loaded;
                }
                else
                {
                    container.CollisionShape = null;
                    container.Status = LoadStatus.Failed;
                }
            }
            
            container.WaitHandle.Set();

            return shape;
        }

        /// <summary>
        /// Gets a <see cref="IcarianEngine.Physics.Shapes.CollisionShape" /> from the given <see cref="IcarianEngine.Definitions.CollisionShapeDef" />
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.CollisionShapeDef" /> to get the <see cref="IcarianEngine.Physics.Shapes.CollisionShape" /> from</param>
        /// <returns>The <see cref="IcarianEngine.Physics.Shapes.CollisionShape" /> if it was loaded successfully, null otherwise.</returns>
        /// @see IcarianEngine.Physics.Shapes.CollisionShape.FromDef
        public static CollisionShape GetCollisionShape(CollisionShapeDef a_def)
        {
            if (a_def == null)
            {
                Logger.IcarianWarning("Null CollisionShapeDef");

                return null;
            }

            if (s_collisionShapes.ContainsKey(a_def.DefName))
            {
                return WaitContainer<CollisionShape>(s_collisionShapes[a_def.DefName]);
            }

            s_collisionShapes.TryAdd(a_def.DefName, new CollisionShapeContainer());

            return GetCollisionShapeInternal(a_def, out LoadStatus _);
        }
        /// <summary>
        /// Gets a <see cref="IcarianEngine.Physics.Shapes.CollisionShape" / from the given <see cref="IcarianEngine.Definitions.CollisionShapeDef" /> asynchronously
        /// </summary>
        /// Lifetime managed by AssetLibrary
        /// <param name="a_def">The <see cref="IcarianEngine.Definitions.CollisionShapeDef" /> to get the <see cref="IcarianEngine.Physics.Shapes.CollisionShape" />  from</param>
        /// <param name="a_callback">The callback to call when the <see cref="IcarianEngine.Physics.Shapes.CollisionShape" />  is loaded</param>
        /// <param name="a_priority">The priority of the job</param>
        /// @see IcarianEngine.Physics.Shapes.CollisionShape.FromDef
        public static void GetCollisionShapeAsync(CollisionShapeDef a_def, GetCollisionShapeCallback a_callback, JobPriority a_priority = JobPriority.Medium)
        {
            if (a_def == null)
            {
                Logger.IcarianWarning("Null CollisionShapeDef");

                if (a_callback != null)
                {
                    a_callback(null, LoadStatus.Failed);
                }

                return;
            }

            s_collisionShapes.TryAdd(a_def.DefName, new CollisionShapeContainer());

            ThreadPool.PushJob(() =>
            {
                LoadStatus status;

                CollisionShape shape = GetCollisionShapeInternal(a_def, out status);

                if (a_callback != null)
                {
                    a_callback(shape, status);
                }
            }, a_priority);
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