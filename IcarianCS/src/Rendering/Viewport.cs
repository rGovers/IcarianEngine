using IcarianEngine.Maths;
using System.Runtime.InteropServices;

namespace IcarianEngine.Rendering
{
    [StructLayout(LayoutKind.Explicit, Pack = 0)]
    public struct Viewport
    {

        [FieldOffset(0)]
        public Vector2 Position;
        [FieldOffset(8)]
        public Vector2 Size;
        [FieldOffset(16)]
        public float MinDepth;
        [FieldOffset(20)]
        public float MaxDepth;
    }
}