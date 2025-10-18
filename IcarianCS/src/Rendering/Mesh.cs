// Icarian Engine - C# Game Engine
//
// License at end of file.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#ifdef ENABLE_STACKTRACE
using System.Diagnostics;
#endif

namespace IcarianEngine.Rendering
{
    public class Mesh : IDestroy
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromFile(string a_path, uint a_modelIndex);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint GenerateFromModel(Array a_vertices, uint[] a_indices, ushort a_vertexSize, float a_radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void DestroyMesh(uint a_addr);

        uint m_bufferAddr = uint.MaxValue;

#ifdef ENABLE_STACKTRACE
        StackTrace m_stackTrace;
#endif

        /// <summary>
        /// Whether or not the Mesh was Disposed/Finalized
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

        internal Mesh(uint a_addr)
        {
            m_bufferAddr = a_addr;

#ifdef ENABLE_STACKTRACE
            m_stackTrace = new StackTrace(true);
#endif
        }

        /// <summary>
        /// Loads a Mesh from a file
        /// </summary>
        /// <param name="a_path">The path the to Mesh</param>
        /// <param name="a_modelIndex">The Model index to load in the file. byte.MaxValue to load all</param>
        /// <returns>The Mesh. Null on failure</returns>
        /// Uses Type <see cref="IcarianEngine.Rendering.Vertex" /> for the Mesh.
        /// Supported formats:
        ///     .obj,
        ///     .fbx,
        ///     .dae,
        ///     .gltf,
        ///     .glb
        /// @see IcarianEngine.AssetLibrary.LoadMesh
        /// @see IcarianEngine.Rendering::Vertex
        public static Mesh LoadMesh(string a_path, byte a_modelIndex = byte.MaxValue)
        {
            uint addr = GenerateFromFile(a_path, (uint)a_modelIndex);
            if (addr != uint.MaxValue)
            {
                return new Mesh(addr);
            }

            Logger.IcarianError($"Mesh failed to load: {a_path}");

            return null;
        }

        /// <summary>
        /// Creates a Mesh from Model data
        /// </summary>
        /// <param name="a_vertices">The vertices to use to generate the Mesh</param>
        /// <param name="a_indices">The indices to use to generate the Mesh</param>
        /// <param name="a_radius">The size of the Mesh to use for culling</param>
        /// <returns>The Mesh. Null on failure</returns>
        public static Mesh FromModel<T>(T[] a_vertices, uint[] a_indices, float a_radius) where T : struct
        {
            uint addr = GenerateFromModel(a_vertices, a_indices, (ushort)Marshal.SizeOf<T>(), a_radius);
            if (addr != uint.MaxValue)
            {
                return new Mesh(addr);
            }

            Logger.IcarianError("Mesh failed to create");

            return null;
        }

        /// <summary>
        /// Disposes of the Mesh
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }
        /// <summary>
        /// Called when the Mesh is being Disposed/Finalized
        /// </summary>
        /// <param name="a_disposing">Determines if it was called from Dispose</param>
        protected virtual void Dispose(bool a_disposing)
        {
            if(m_bufferAddr != uint.MaxValue)
            {
                if(a_disposing)
                {
                    DestroyMesh(m_bufferAddr);
                }
                else
                {
                    Logger.IcarianError("Mesh not Disposed");

#ifdef ENABLE_STACKTRACE
                    CallStack.PrintStackTrace(m_stackTrace);
#endif
                }

                m_bufferAddr = uint.MaxValue;
            }
            else
            {
                Logger.IcarianWarning("Mesh already Disposed");
            }
        }
        ~Mesh()
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
