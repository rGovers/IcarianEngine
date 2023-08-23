using IcarianEngine.Maths;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    public struct Vertex
    {
        public Vector4 Position;
        public Vector3 Normal;
        public Vector4 Color;
        public Vector2 TexCoords;

        public static VertexInputAttribute[] GetAttributes()
        {
            VertexInputAttribute[] attributes = new VertexInputAttribute[4];
            attributes[0].Location = 0;
            attributes[0].Type = VertexType.Float;
            attributes[0].Count = 4;
            attributes[0].Offset = (ushort)Marshal.OffsetOf<Vertex>("Position");

            attributes[1].Location = 1;
            attributes[1].Type = VertexType.Float;
            attributes[1].Count = 3;
            attributes[1].Offset = (ushort)Marshal.OffsetOf<Vertex>("Normal");

            attributes[2].Location = 2;
            attributes[2].Type = VertexType.Float;
            attributes[2].Count = 4;
            attributes[2].Offset = (ushort)Marshal.OffsetOf<Vertex>("Color");

            attributes[3].Location = 3;
            attributes[3].Type = VertexType.Float;
            attributes[3].Count = 2;
            attributes[3].Offset = (ushort)Marshal.OffsetOf<Vertex>("TexCoords");

            return attributes;
        }
    }

    public enum VertexType : ushort
    {
        Null = ushort.MaxValue,
        Float = 0,
        Int = 1,
        UInt = 2
    };

    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    public struct VertexInputAttribute
    {
        public ushort Location;
        public VertexType Type;
        public ushort Count;
        public ushort Offset;
    };

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

        public static Model CreateModel<T>(T[] a_vertices, uint[] a_indices, float a_radius) where T : struct 
        {
            return new Model(GenerateModel(a_vertices, a_indices, (ushort)Marshal.SizeOf<T>(), a_radius));
        }

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