// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#ifdef ENABLE_STACKTRACE
using System.Diagnostics;
#endif

#include "EngineModelInteropStructures.h"

namespace IcarianEngine.Rendering
{
    public class Model : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static ModelDataStructure GetModelData(string a_path, uint a_index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateModel(Array a_vertices, uint[] a_indices, ushort a_vertexSize, float a_radius); 
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path, uint a_modelIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateSkinnedFromFile(string a_path, uint a_modelIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyModel(uint a_addr);

        uint       m_bufferAddr = uint.MaxValue;

#ifdef ENABLE_STACKTRACE
        StackTrace m_stackTrace;
#endif

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

        internal Model(uint a_addr)
        {
            m_bufferAddr = a_addr;

#ifdef ENABLE_STACKTRACE
            m_stackTrace = new StackTrace(true);
#endif
        }

        /// <summary>
        /// Creates a Model from a set of vertices and indices
        /// </summary>
        /// <typeparam name="T">The type of vertex</typeparam>
        /// <param name="a_vertices">The vertices</param>
        /// <param name="a_indices">The indices</param>
        /// <param name="a_radius">The radius of the model. Used for frustum culling. Ignored if there is no transform.</param>
        /// <returns>The model. Null on failure.</returns>
        public static Model CreateModel<T>(T[] a_vertices, uint[] a_indices, float a_radius) where T : struct 
        {
            if (a_vertices == null)
            {
                Logger.IcarianError("Creating Model with null vertices");

                return null;
            }

            if (a_indices == null)
            {
                Logger.IcarianError("Creating Model with null indices");

                return null;
            }

            ushort vertexSize = (ushort)Marshal.SizeOf<T>();
            if (vertexSize <= 0)
            {
                Logger.IcarianError("Creating Model with size 0 vertex");

                return null;
            }

            uint addr = GenerateModel(a_vertices, a_indices, (ushort)Marshal.SizeOf<T>(), a_radius);
            if (addr != uint.MaxValue)
            {
                return new Model(addr);
            }

            Logger.IcarianError("Model Failed to create");

            return null;
        }

        /// <summary>
        /// Loads Model data from a file
        /// </summary>
        /// <param name="a_path">The path to the Model</param>
        /// <param name="a_modelIndex">The Model index to load in the file</param>
        /// <param name="a_vertices">The <see cref="IcarianEngine.Rendering.Vertex" /> data of the loaded Model</param>
        /// <param name="a_indices">The index data of the loaded Model</param>
        /// <returns>If the data loaded successfully</returns>
        /// Supported formats: 
        ///     .obj,
        ///     .fbx,
        ///     .dae,
        ///     .gltf,
        ///     .glb
        public static bool LoadModelData(string a_path, byte a_modelIndex, out Vertex[] a_vertices, out uint[] a_indices)
        {
            a_vertices = null;
            a_indices = null;

            ModelDataStructure dat = GetModelData(a_path, (uint)a_modelIndex);
            if (dat.Vertices != null && dat.Indices != null)
            {
                a_vertices = dat.Vertices;
                a_indices = dat.Indices;

                return true;
            }

            return false;
        }

        /// <summary>
        /// Loads a Model from a file
        /// </summary>
        /// <param name="a_path">The path to the Model</param>
        /// <param name="a_modelIndex">The <odel index to load in the file</param>
        /// <returns>The Model. Null on failure.</returns>
        /// Uses Type <see cref="Icarianengine.Rendering.Vertex" /> for the model.
        /// Supported formats: 
        ///     .obj,
        ///     .fbx,
        ///     .dae,
        ///     .gltf,
        ///     .glb
        /// @see IcarianEngine.AssetLibrary.LoadModel
        /// @see IcarianEngine.Rendering::Vertex
        public static Model LoadModel(string a_path, byte a_modelIndex = byte.MaxValue)
        {
            uint addr = GenerateFromFile(a_path, (uint)a_modelIndex);
            if (addr != uint.MaxValue)
            {
                return new Model(addr);
            }

            Logger.IcarianError($"Model Failed to load: {a_path}");

            return null;
        }

        /// <summary>
        /// Loads a Skinned Model from a file
        /// </summary>
        /// <param name="a_path">The path to the model</param>
        /// <param name="a_modelIndex">The <odel index to load in the file</param>
        /// <returns>The Model. Null on failure.</returns>
        /// Uses Type <see cref="IcarianEngine.Rendering.SkinnedVertex" /> for the Model
        /// Supported formats:
        ///     .dae,
        ///     .fbx,
        ///     .gltf,
        ///     .glb
        /// @see IcarianEngine.AssetLibrary.LoadSkinnedModel
        /// @see IcarianEngine.Rendering::SkinnedVertex
        public static Model LoadSkinnedModel(string a_path, byte a_modelIndex = byte.MaxValue)
        {
            uint addr = GenerateSkinnedFromFile(a_path, (uint)a_modelIndex);
            if (addr != uint.MaxValue)
            {
                return new Model(addr);
            }

            Logger.IcarianError($"Model Skinned Failed to load: {a_path}");

            return null;
        }

        /// <summary>
        /// Disposes of the Model
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Called when the Model is being Disposed/Finalized
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
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
                    Logger.IcarianError("Model not Disposed");

#ifdef ENABLE_STACKTRACE
                    CallStack.PrintStackTrace(m_stackTrace);
#endif  
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("Model already Disposed");
            }
        }

        ~Model()
        {
            Dispose(false);
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
