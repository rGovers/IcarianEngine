// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Audio;
using IcarianEngine.Mod;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
using IcarianEngine.Rendering.Shaders;
using System;
using System.Collections.Concurrent;

#ifdef ENABLE_EXPERIMENTAL
using IcarianEngine.Rendering.Video;
#endif

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

    public static partial class AssetLibrary
    {
        static ConcurrentDictionary<string, AudioClipContainer>             s_audioClips;
#ifdef ENABLE_EXPERIMENTAL
        static ConcurrentDictionary<string, VideoClipContainer>             s_videoClips;
 #endif

        static ConcurrentDictionary<string, MaterialContainer>              s_materials;
        static ConcurrentDictionary<string, GraphicsComputeShaderContainer> s_graphicsComputeShaders;
        static ConcurrentDictionary<string, MeshShaderContainer>            s_meshShaders;
        static ConcurrentDictionary<string, VertexShaderContainer>          s_vertexShaders;
        static ConcurrentDictionary<string, PixelShaderContainer>           s_pixelShaders;

        static ConcurrentDictionary<string, TextureContainer>               s_textures;
        static ConcurrentDictionary<string, TextureSamplerContainer>        s_textureSamplers;

        static ConcurrentDictionary<string, ModelContainer>                 s_models;
        static ConcurrentDictionary<string, ModelContainer>                 s_skinnedModels;

        static ConcurrentDictionary<string, MeshContainer>                  s_meshes;

        static ConcurrentDictionary<string, AnimationClipContainer>         s_animationClips;

        static ConcurrentDictionary<string, SkeletonContainer>              s_skeletons;

        static ConcurrentDictionary<string, FontContainer>                  s_fonts;

        static ConcurrentDictionary<string, CollisionShapeContainer>        s_collisionShapes;

        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Audio.AudioClip" /> async
        /// </summary>
        public delegate void LoadAudioClipCallback(AudioClip a_clip, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <cee cref="IcarianEngine.Rendering.Shaders.ComputeShader" /> async in in Graphics mode
        /// </summary>
        public delegate void LoadGraphicsComputeShaderCallback(ComputeShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Shaders.MeshShader" /> async
        /// </summary>
        public delegate void LoadMeshShaderCallback(MeshShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Shaders.VertexShader" /> async
        /// </summary>
        public delegate void LoadVertexShaderCallback(VertexShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Shaders.PixelShader" /> async
        /// </summary>
        public delegate void LoadPixelShaderCallback(PixelShader a_shader, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Font" /> async
        /// </summary>
        public delegate void LoadFontCallback(Font a_font, LoadStatus a_status);
        /// <summary>
        /// Delegate for loading a <see cref="IcarianEngine.Rendering.Mesh" /> async
        /// </summary>
        public delegate void LoadMeshCallback(Mesh a_model, LoadStatus a_status);
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

            s_graphicsComputeShaders = new ConcurrentDictionary<string, GraphicsComputeShaderContainer>();
            s_meshShaders = new ConcurrentDictionary<string, MeshShaderContainer>();
            s_vertexShaders = new ConcurrentDictionary<string, VertexShaderContainer>();
            s_pixelShaders = new ConcurrentDictionary<string, PixelShaderContainer>();

            s_textures = new ConcurrentDictionary<string, TextureContainer>();
            s_textureSamplers = new ConcurrentDictionary<string, TextureSamplerContainer>();

            s_models = new ConcurrentDictionary<string, ModelContainer>();
            s_skinnedModels = new ConcurrentDictionary<string, ModelContainer>();

            s_meshes = new ConcurrentDictionary<string, MeshContainer>();

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

            foreach (GraphicsComputeShaderContainer cShader in s_graphicsComputeShaders.Values)
            {
                if (cShader.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (cShader.Status != LoadStatus.Loaded)
                {
                    cShader.WaitHandle.WaitOne();
                }

                if (cShader.Shader != null && !cShader.Shader.IsDisposed)
                {
                    cShader.Shader.Dispose();
                }
            }
            foreach (MeshShaderContainer mShader in s_meshShaders.Values)
            {
                if (mShader.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (mShader.Status != LoadStatus.Loaded)
                {
                    mShader.WaitHandle.WaitOne();
                }

                if (mShader.Shader != null && !mShader.Shader.IsDisposed)
                {
                    mShader.Shader.Dispose();
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

            foreach (MeshContainer mesh in s_meshes.Values)
            {
                if (mesh.Status == LoadStatus.Failed)
                {
                    continue;
                }

                if (mesh.Status != LoadStatus.Loaded)
                {
                    mesh.WaitHandle.WaitOne();
                }

                if (mesh.Mesh != null && !mesh.Mesh.IsDisposed)
                {
                    mesh.Mesh.Dispose();
                }
            }
            s_meshes.Clear();

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
