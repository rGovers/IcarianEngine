using IcarianEngine.Maths;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine.Physics
{
    [StructLayout(LayoutKind.Sequential, Pack = 0)]
    struct RaycastResultS
    {
        public Vector3 Position;
        public uint BodyAddr;
    };

    public struct RaycastResult
    {
        public Vector3 Position;
        public PhysicsBody Body;
    };

    public static class Physics
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void SetGravity(Vector3 a_gravity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern Vector3 GetGravity();

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern RaycastResultS[] RaycastS(Vector3 a_pos, Vector3 a_dir);

        public static Vector3 Gravity
        {
            get
            {
                return GetGravity();
            }
            set
            {
                SetGravity(value);
            }
        }

        public static bool Raycast(Vector3 a_pos, Vector3 a_dir, out RaycastResult[] a_hits)
        {
            a_hits = null;

            RaycastResultS[] result = RaycastS(a_pos, a_dir);
            if (result != null)
            {
                int count = result.Length;

                a_hits = new RaycastResult[count];

                for (int i = 0; i < count; ++i)
                {
                    a_hits[i].Position = result[i].Position;
                    a_hits[i].Body = PhysicsBody.GetBody(result[i].BodyAddr);
                }

                return true;
            }

            return false;
        }
    }
}