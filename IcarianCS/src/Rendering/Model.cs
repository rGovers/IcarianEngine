using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EngineModelInteropStructures.h"

namespace IcarianEngine.Rendering
{
    public class Model : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateModel(Array a_vertices, uint[] a_indices, ushort a_vertexSize, float a_radius); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateSkinnedFromFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyModel(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

        /// <summary>
        /// Whether the model has been disposed
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

        Model(uint a_addr)
        {
            m_bufferAddr = a_addr;
        }

        /// <summary>
        /// Creates a model from a set of vertices and indices
        /// </summary>
        /// <typeparam name="T">The type of vertex</typeparam>
        /// <param name="a_vertices">The vertices</param>
        /// <param name="a_indices">The indices</param>
        /// <param name="a_radius">The radius of the model. Used for frustum culling. Ignored if there is no transform.</param>
        /// <returns>The model. Null on failure.</returns>
        public static Model CreateModel<T>(T[] a_vertices, uint[] a_indices, float a_radius) where T : struct 
        {
            uint addr = GenerateModel(a_vertices, a_indices, (ushort)Marshal.SizeOf<T>(), a_radius);
            if (addr != uint.MaxValue)
            {
                return new Model(addr);
            }

            Logger.IcarianError("Model Failed to create");

            return null;
        }

        /// <summary>
        /// Loads a model from a file
        /// </summary>
        /// <param name="a_path">The path to the model</param>
        /// <returns>The model. Null on failure.</returns>
        /// Uses Type Vertex for the model.
        /// Supported formats: 
        ///     .obj,
        ///     .fbx,
        ///     .dae
        ///     .gltf
        ///     .glb
        /// @see IcarianEngine.AssetLibrary.LoadModel
        /// @see IcarianEngine.Rendering::Vertex
        public static Model LoadModel(string a_path)
        {
            uint addr = GenerateFromFile(a_path);
            if (addr != uint.MaxValue)
            {
                return new Model(addr);
            }

            Logger.IcarianError($"Model Failed to load: {a_path}");

            return null;
        }
        /// <summary>
        /// Loads a skinned model from a file
        /// </summary>
        /// <param name="a_path">The path to the model</param>
        /// <returns>The model. Null on failure.</returns>
        /// Uses Type SkinnedVertex for the model.
        /// Supported formats:
        ///     .dae,
        ///     .fbx
        /// @see IcarianEngine.AssetLibrary.LoadSkinnedModel
        /// @see IcarianEngine.Rendering::SkinnedVertex
        public static Model LoadSkinnedModel(string a_path)
        {
            uint addr = GenerateSkinnedFromFile(a_path);
            if (addr != uint.MaxValue)
            {
                return new Model(addr);
            }

            Logger.IcarianError($"Model Skinned Failed to load: {a_path}");

            return null;
        }

        /// <summary>
        /// Disposes the model
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the model is being disposed
        /// </summary>
        /// <param name="a_disposing">Whether the model is being disposed</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyModel(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianWarning("Model Failed to Dispose");
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianError("Multiple Model Dispose");
            }
        }

        ~Model()
        {
            Dispose(false);
        }
    }
}