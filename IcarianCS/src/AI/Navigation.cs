using IcarianEngine.Maths;
using System.Runtime.CompilerServices;

namespace IcarianEngine.AI
{
    public static class Navigation
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern static Vector3[] GetPath(Vector3 a_startPoint, Vector3 a_endPoint);
    }
}