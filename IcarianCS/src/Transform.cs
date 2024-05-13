using IcarianEngine.Maths;
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineTransformInterop.h"
#include "EngineTransformInteropStructures.h"
#include "InteropBinding.h"

ENGINE_TRANSFORM_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine
{
    public class Transform : IDestroy
    {      
        uint            m_bufferAddr = uint.MaxValue;
      
        GameObject      m_object;

        Transform       m_parent;
        List<Transform> m_children;

        /// <summary>
        /// Whether the Transform has been Disposed
        /// </summary>
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

        /// <summary>
        /// The parent of the Transform
        /// </summary>
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

                TransformBuffer buffer = TransformInterop.GetTransformBuffer(m_bufferAddr);

                if (m_parent != null)
                {
                    m_parent.m_children.Add(this);

                    buffer.ParentAddr = m_parent.m_bufferAddr;
                }
                else
                {
                    buffer.ParentAddr = uint.MaxValue;
                }

                TransformInterop.SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// The children of the Tranform
        /// </summary>
        public IEnumerable<Transform> Children
        {
            get
            {
                return m_children;
            }
        }

        /// <summary>
        /// The <see cref="IcarianEngine.GameObject" /> the Tranform is attached to
        /// </summary>
        public GameObject Object
        {
            get
            {
                return m_object;
            }
        }

        /// <summary>
        /// The translation/position of the Transform
        /// </summary>
        public Vector3 Translation
        {
            get
            {
                return TransformInterop.GetTransformBuffer(m_bufferAddr).Translation;
            }
            set
            {
                TransformBuffer buffer = TransformInterop.GetTransformBuffer(m_bufferAddr);
                buffer.Translation = value;
                TransformInterop.SetTransformBuffer(m_bufferAddr, buffer);
            }
        }
        /// <summary>
        /// The rotation of the Transform as a <see cref="IcarianEngine.Maths.Quaternion" />
        /// </summary>
        public Quaternion Rotation
        {
            get
            {
                return TransformInterop.GetTransformBuffer(m_bufferAddr).Rotation;
            }
            set
            {
                TransformBuffer buffer = TransformInterop.GetTransformBuffer(m_bufferAddr);
                buffer.Rotation = value;
                TransformInterop.SetTransformBuffer(m_bufferAddr, buffer);
            }
        }
        /// <summary>
        /// The scale of the Transform
        /// </summary>
        public Vector3 Scale
        {
            get
            {
                return TransformInterop.GetTransformBuffer(m_bufferAddr).Scale;
            }
            set
            {
                TransformBuffer buffer = TransformInterop.GetTransformBuffer(m_bufferAddr);
                buffer.Scale = value;
                TransformInterop.SetTransformBuffer(m_bufferAddr, buffer);
            }
        }

        /// <summary>
        /// The forward vector of the Tranform
        /// </summary>
        public Vector3 Forward
        {
            get
            {
                return Rotation * Vector3.Forward;
            }
        }
        /// <summary>
        /// The up vector of the Transform
        /// </summary>
        public Vector3 Up
        {
            get
            {
                return Rotation * Vector3.Up;
            }
        }
        /// <summary>
        /// The right vector of the Transform
        /// </summary>
        public Vector3 Right
        {
            get
            {
                return Rotation * Vector3.Right;
            }
        }

        /// <summary>
        /// Sets the Transform to a transformation <see cref="IcarianEngine.Maths.Matrix4" />
        /// </summary>
        /// <param name="a_mat">The matrix to set the Transform to</param>
        public void SetMatrix(Matrix4 a_mat)
        {
            TransformInterop.SetTransformMatrix(m_bufferAddr, a_mat.ToArray());
        }

        /// <summary>
        /// Turns the Transform into a transformation <see cref="IcarianEngine.Maths.Matrix4" />
        /// </summary>
        public Matrix4 ToMatrix()
        {
            float[] matrix = TransformInterop.GetTransformMatrix(m_bufferAddr);

            return new Matrix4(matrix[0],  matrix[1],  matrix[2],  matrix[3],
                               matrix[4],  matrix[5],  matrix[6],  matrix[7],
                               matrix[8],  matrix[9],  matrix[10], matrix[11],
                               matrix[12], matrix[13], matrix[14], matrix[15]);
        }
        /// <summary>
        /// Turns the Transform into a global tranformation <see cref="IcarianEngine.Maths.Matrix4" />
        /// </summary>
        public Matrix4 ToGlobalMatrix()
        {
            float[] matrix = TransformInterop.GetGlobalTransformMatrix(m_bufferAddr);

            return new Matrix4(matrix[0],  matrix[1],  matrix[2],  matrix[3],
                               matrix[4],  matrix[5],  matrix[6],  matrix[7],
                               matrix[8],  matrix[9],  matrix[10], matrix[11],
                               matrix[12], matrix[13], matrix[14], matrix[15]);
        }

        Transform()
        {
            m_object = null;
            m_children = new List<Transform>();
        }
        internal Transform(GameObject a_object)
        {
            m_object = a_object;
            m_children = new List<Transform>();

            m_bufferAddr = TransformInterop.GenerateTransformBuffer();
        }

        internal static Transform[] BatchGenerateTransforms(GameObject[] a_gameObjects)
        {
            uint size = (uint)a_gameObjects.LongLength;

            Transform[] transforms = new Transform[size];
            uint[] addrs = TransformInterop.BatchGenerateTransformBuffer(size);

            for (uint i = 0; i < size; ++i)
            {
                Transform t = new Transform();

                t.m_bufferAddr = addrs[i];
                t.m_object = a_gameObjects[i];

                transforms[i] = t;
            }

            return transforms;
        }

        /// <summary>
        /// Disposes of the Transform
        /// <summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Transform is being Disposed
        /// </summary>
        /// <param name="a_disposing">Whether the Transform is being Disposed or Finalized</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    TransformInterop.DestroyTransformBuffer(m_bufferAddr);
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