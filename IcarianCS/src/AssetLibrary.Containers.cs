// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Audio;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
using IcarianEngine.Rendering.UI;
#ifdef ENABLE_EXPERIMENTAL
using IcarianEngine.Rendering.Video;
#endif
using System.Threading;

namespace IcarianEngine
{
    /// @cond INTERNAL

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
                Clip = value as AudioClip;
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

#ifdef ENABLE_EXPERIMENTS
    class VideoClipContainer : IAssetContainer
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
        public VideoClip Clip
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
                Clip = value as VideoClip;
            }
        }

        public VideoClipContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Clip = null;
        }

        public object LoadValue(string a_input)
        {
            return VideoClip.LoadVideoClip(a_input);
        }
    }
#endif

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
                Shader = value as VertexShader;
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
                Shader = value as PixelShader;
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

    class FontContainer : IAssetContainer
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
        public Font Font
        {
            get;
            set;
        }

        public object Value
        {
            get
            {
                return Font;
            }
            set
            {
                Font = value as Font;
            }
        }

        public FontContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Font = null;
        }

        public object LoadValue(string a_input)
        {
            return Font.LoadFont(a_input);
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
                Model = value as Model;
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
            return null;
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
                Texture = value as Texture;
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
    class TextureSamplerContainer : IAssetContainer
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
        public TextureSampler Sampler
        {
            get;
            set;
        }

        public object Value
        {
            get
            {
                return Sampler;
            }
            set
            {
                Sampler = value as TextureSampler;
            }
        }

        public TextureSamplerContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            Sampler = null;
        }

        public object LoadValue(string a_input)
        {
            return null;
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

    class CollisionShapeContainer : IAssetContainer
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
        public CollisionShape CollisionShape
        {
            get;
            set;
        }

        public object Value
        {
            get
            {
                return CollisionShape;
            }
            set
            {
                CollisionShape = (CollisionShape)value;
            }
        }

        public CollisionShapeContainer()
        {
            Status = LoadStatus.Unloaded;
            WaitHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
            CollisionShape = null;
        }

        public object LoadValue(string a_input)
        {
            return null;
        }
    }

    /// @endcond
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