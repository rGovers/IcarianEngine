using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine
{
    public static class Random
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void FillBuffer(byte[] a_buffer, uint a_offset);

        const uint BufferSize = 512;

        static byte[] s_buffer = new byte[BufferSize];
        static uint s_bufferIndex = BufferSize;

        static void FillBuffer()
        {
            uint offset = BufferSize - s_bufferIndex;
            for (int i = 0; i < offset; ++i)
            {
                s_buffer[i] = s_buffer[i + s_bufferIndex];
            }

            FillBuffer(s_buffer, offset);

            s_bufferIndex = 0;
        }

        public static uint Range(uint a_min, uint a_max)
        {
            if (s_bufferIndex + 4 > BufferSize)
            {
                FillBuffer();
            }

            uint value = BitConverter.ToUInt32(s_buffer, (int)s_bufferIndex);

            s_bufferIndex += 4;

            return a_min + (value % (a_max - a_min));
        }
        public static float Range(float a_min, float a_max)
        {
            if (s_bufferIndex + 4 > BufferSize)
            {
                FillBuffer();
            }

            uint value = BitConverter.ToUInt32(s_buffer, (int)s_bufferIndex);

            s_bufferIndex += 4;

            return (float)(a_min + (value / (double)uint.MaxValue) * (a_max - a_min));
        }
        public static int Range(int a_min, int a_max)
        {
            if (s_bufferIndex + 4 > BufferSize)
            {
                FillBuffer();
            }

            int value = BitConverter.ToInt32(s_buffer, (int)s_bufferIndex);

            s_bufferIndex += 4;

            return a_min + (value % (a_max - a_min));
        }
    }
}