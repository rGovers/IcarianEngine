using IcarianEngine.Audio;
using IcarianEngine.Physics.Shapes;
using IcarianEngine.Rendering;
using IcarianEngine.Rendering.Animation;
using IcarianEngine.Rendering.UI;
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