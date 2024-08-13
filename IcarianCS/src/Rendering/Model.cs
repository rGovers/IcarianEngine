// Icarian Engine - C# Game Engine
// 
// License at end of file.

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
        extern static uint GenerateFromFile(string a_path, uint a_modelIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateSkinnedFromFile(string a_path, uint a_modelIndex);
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

        internal Model(uint a_addr)
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
        /// Loads a skinned model from a file
        /// </summary>
        /// <param name="a_path">The path to the model</param>
        /// <returns>The model. Null on failure.</returns>
        /// Uses Type SkinnedVertex for the model.
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