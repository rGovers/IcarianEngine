using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    internal struct TransformBuffer
    {
        public uint ParentIndex;

        public Vector3 Translation;
        public Quaternion Rotation;
        public Vector3 Scale;
    }

    public class Transform : IDestroy
    {      
        uint            m_bufferAddr = uint.MaxValue;
      
        GameObject      m_object;

        Transform       m_parent;
        List<Transform> m_children;

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateTransformBuffer();
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static TransformBuffer GetTransformBuffer(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetTransformBuffer(uint a_addr, TransformBuffer a_buffer);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyTransformBuffer(uint a_addr); 

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float[] GetTransformMatrix(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static float[] GetGlobalTransformMatrix(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void SetTransformMatrix(uint a_addr, float[] a_matrix);

        public bool IsDisposed
        {
            get
            {
                return m_bufferAddr == uint.MaxValue;
            }
        }

        internal uint InternalAddr
        {
            get
            {
                return m_bufferAddr;
            }
        }

        public Transform Parent
        {
            get
            {
                return m_parent;
            }
            set
            {
                if (m_parent != null)
                {
                    m_parent.m_children.Remove(this);
                }

                m_parent = value;

                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);

                if (m_parent != null)
                {
                    m_parent.m_children.Add(this);

                    buffer.ParentIndex = m_parent.m_bufferAddr;
                }
                else
                {
                    buffer.ParentIndex = uint.MaxValue;
                }

                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public IEnumerable<Transform> Children
        {
            get
            {
                return m_children;
            }
        }

        public GameObject Object
        {
            get
            {
                return m_object;
            }
        }

        public Vector3 Translation
        {
            get
            {
                return GetTransformBuffer(m_bufferAddr).Translation;
            }
            set
            {
                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);
                buffer.Translation = value;
                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public Quaternion Rotation
        {
            get
            {
                return GetTransformBuffer(m_bufferAddr).Rotation;
            }
            set
            {
                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);
                buffer.Rotation = value;
                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public Vector3 Scale
        {
            get
            {
                return GetTransformBuffer(m_bufferAddr).Scale;
            }
            set
            {
                TransformBuffer buffer = GetTransformBuffer(m_bufferAddr);
                buffer.Scale = value;
                SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        public Vector3 Forward
        {
            get
            {
                return Rotation * Vector3.Forward;
            }
        }
        public Vector3 Up
        {
            get
            {
                return Rotation * Vector3.Up;
            }
        }
        public Vector3 Right
        {
            get
            {
                return Rotation * Vector3.Right;
            }
        }

        public void SetMatrix(Matrix4 a_mat)
        {
            SetTransformMatrix(m_bufferAddr, a_mat.ToArray());
        }

        public Matrix4 ToMatrix()
        {
            float[] matrix = GetTransformMatrix(m_bufferAddr);

            return new Matrix4(matrix[0],  matrix[1],  matrix[2],  matrix[3],
                               matrix[4],  matrix[5],  matrix[6],  matrix[7],
                               matrix[8],  matrix[9],  matrix[10], matrix[11],
                               matrix[12], matrix[13], matrix[14], matrix[15]);
        }
        public Matrix4 ToGlobalMatrix()
        {
            float[] matrix = GetGlobalTransformMatrix(m_bufferAddr);

            return new Matrix4(matrix[0],  matrix[1],  matrix[2],  matrix[3],
                               matrix[4],  matrix[5],  matrix[6],  matrix[7],
                               matrix[8],  matrix[9],  matrix[10], matrix[11],
                               matrix[12], matrix[13], matrix[14], matrix[15]);
        }

        internal Transform(GameObject a_object)
        {
            m_object = a_object;
            m_children = new List<Transform>();

            m_bufferAddr = GenerateTransformBuffer();
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
                    DestroyTransformBuffer(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Transform Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Transform Dispose");
            }
        }

        ~Transform()
        {
            Dispose(false);
        }
    }
}