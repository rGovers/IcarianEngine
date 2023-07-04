using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System;
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

    public class Camera : Component, IDestroy
    {
        static Dictionary<uint, Camera> s_bufferLookup = new Dictionary<uint, Camera>();

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
        extern static float[] GetProjectionMatrix(uint a_addr, uint a_width, uint a_height);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float[] GetProjectionMatrixNF(uint a_addr, uint a_width, uint a_height, float a_near, float a_far);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static Vector3 ScreenToWorld(uint a_addr, Vector3 a_screenPos, Vector2 a_screenSize);

        public CameraDef CameraDef
        {
            get
            {
                return Def as CameraDef;
            }
        }

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

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

        public override void Init()
        {
            m_bufferAddr = GenerateBuffer(Transform.InternalAddr);

            s_bufferLookup.Add(m_bufferAddr, this);
            
            CameraDef def = CameraDef;
            if (def != null)
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
        public Matrix4 ToProjection(uint a_width, uint a_height)
        {
            float[] matrix = GetProjectionMatrix(m_bufferAddr, a_width, a_height);
            
            return new Matrix4(matrix[0],  matrix[1],  matrix[2],  matrix[3],
                               matrix[4],  matrix[5],  matrix[6],  matrix[7],
                               matrix[8],  matrix[9],  matrix[10], matrix[11],
                               matrix[12], matrix[13], matrix[14], matrix[15]);
        }
        public Matrix4 ToProjection(uint a_width, uint a_height, float a_near, float a_far)
        {
            float[] matrix = GetProjectionMatrixNF(m_bufferAddr, a_width, a_height, a_near, a_far);
            
            return new Matrix4(matrix[0],  matrix[1],  matrix[2],  matrix[3],
                               matrix[4],  matrix[5],  matrix[6],  matrix[7],
                               matrix[8],  matrix[9],  matrix[10], matrix[11],
                               matrix[12], matrix[13], matrix[14], matrix[15]);
        }

        internal static Camera GetCamera(uint a_buffer)
        {
            if (s_bufferLookup.ContainsKey(a_buffer))
            {
                return s_bufferLookup[a_buffer];
            }

            return null;
        }

        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    CameraDef def = CameraDef;
                    if (def != null)
                    {
                        if (def.RenderTexture.Width != uint.MaxValue && def.RenderTexture.Height != uint.MaxValue)
                        {
                            RenderTexture.Dispose();
                            RenderTexture = null;
                        }
                    }

                    DestroyBuffer(m_bufferAddr);

                    s_bufferLookup.Remove(m_bufferAddr);
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

        ~Camera()
        {
            Dispose(false);
        }
    }}