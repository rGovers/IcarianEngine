using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct CameraBuffer
    {
        public uint TransformBuffer;
        public uint RenderTexture;
        public Viewport Viewport;
        public uint RenderLayer;
        public float FOV;
        public float Near;
        public float Far;
    };

    public class Camera : GameObject
    {
        static Dictionary<uint, Camera> BufferLookup = new Dictionary<uint, Camera>();

        uint m_bufferAddr = uint.MaxValue;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateBuffer(uint a_transformAddr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static CameraBuffer GetBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetBuffer(uint a_addr, CameraBuffer a_buffer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector3 ScreenToWorld(uint a_addr, Vector3 a_screenPos, Vector2 a_screenSize);

        internal uint BufferAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        public Viewport Viewport
        {
            get
            {
                return GetBuffer(m_bufferAddr).Viewport;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.Viewport = value;

                SetBuffer(m_bufferAddr, val);
            }
        }

        public float FOV
        {
            get
            {
                return GetBuffer(m_bufferAddr).FOV;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.FOV = value;

                SetBuffer(m_bufferAddr, val);
            }
        }
        public float Near
        {
            get
            {
                return GetBuffer(m_bufferAddr).Near;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.Near = value;

                SetBuffer(m_bufferAddr, val);
            }
        }
        public float Far
        {
            get
            {
                return GetBuffer(m_bufferAddr).Far;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.Far = value;

                SetBuffer(m_bufferAddr, val);
            }
        }

        public uint RenderLayer
        {
            get
            {
                return GetBuffer(m_bufferAddr).RenderLayer;
            }
            set
            {
                CameraBuffer val = GetBuffer(m_bufferAddr);

                val.RenderLayer = value;

                SetBuffer(m_bufferAddr, val);
            }
        }

        public IRenderTexture RenderTexture
        {
            get
            {
                uint textureAddr = GetBuffer(m_bufferAddr).RenderTexture;

                return RenderTextureCmd.GetRenderTexture(textureAddr);
            }
            set
            {
                CameraBuffer buffer = GetBuffer(m_bufferAddr);
                buffer.RenderTexture = uint.MaxValue;

                if (value is RenderTexture rVal)
                {
                    buffer.RenderTexture = rVal.BufferAddr;
                }
                else if (value is MultiRenderTexture mVal)
                {
                    buffer.RenderTexture = mVal.BufferAddr;
                }

                SetBuffer(m_bufferAddr, buffer);
            }
        }

        public Camera() : base()
        {            
            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            BufferLookup.Add(m_bufferAddr, this);
        }

        public override void Init()
        {
            base.Init();

            if (Def is CameraDef def)
            {
                uint textureAddr = uint.MaxValue;
                if (def.RenderTexture.Width != uint.MaxValue && def.RenderTexture.Height != uint.MaxValue)
                {
                    if (def.RenderTexture.Count == 1)
                    {
                        RenderTexture texture = new RenderTexture(def.RenderTexture.Width, def.RenderTexture.Height, def.RenderTexture.HDR);
                        textureAddr = texture.BufferAddr;
                    }
                    else
                    {
                        MultiRenderTexture texture = new MultiRenderTexture(def.RenderTexture.Count, def.RenderTexture.Width, def.RenderTexture.Height, def.RenderTexture.HDR);
                        textureAddr = texture.BufferAddr;
                    }
                }

                CameraBuffer val = GetBuffer(m_bufferAddr);
                val.Viewport = def.Viewport;
                val.FOV = def.FOV;
                val.Near = def.Near;
                val.Far = def.Far;
                val.RenderLayer = def.RenderLayer;
                val.RenderTexture = textureAddr;

                SetBuffer(m_bufferAddr, val);
            }
        }

        public Vector3 ScreenToWorld(Vector3 a_screenPos, Vector2 a_screenSize)
        {
            return ScreenToWorld(m_bufferAddr, a_screenPos, a_screenSize);
        }

        internal static Camera GetCamera(uint a_buffer)
        {
            if (BufferLookup.ContainsKey(a_buffer))
            {
                return BufferLookup[a_buffer];
            }

            return null;
        }

        protected override void Dispose(bool a_disposing)
        {
            base.Dispose(a_disposing);

            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    if (Def is CameraDef def)
                    {
                        if (def.RenderTexture.Width != uint.MaxValue && def.RenderTexture.Height != uint.MaxValue)
                        {
                            RenderTexture.Dispose();
                            RenderTexture = null;
                        }
                    }

                    DestroyBuffer(m_bufferAddr);

                    BufferLookup.Remove(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Camera Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Camera Dispose");
            }
        }
    }}