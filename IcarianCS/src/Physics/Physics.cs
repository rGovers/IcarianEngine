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
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern uint[] SphereCollisionS(Vector3 a_pos, float a_radius);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern uint[] BoxCollisionS(float[] a_transform, Vector3 a_extents);
        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern uint[] AABBCollisionS(Vector3 a_min, Vector3 a_max);

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
        public static bool SphereCollision(Vector3 a_pos, float a_radius, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = SphereCollisionS(a_pos, a_radius);
            if (result != null)
            {
                int count = result.Length;

                a_bodies = new PhysicsBody[count];

                for (int i = 0; i < count; ++i)
                {
                    a_bodies[i] = PhysicsBody.GetBody(result[i]);
                }

                return true;
            }

            return false;
        }
        public static bool BoxCollision(Matrix4 a_transform, Vector3 a_extents, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = BoxCollisionS(a_transform.ToArray(), a_extents);
            if (result != null)
            {
                int count = result.Length;

                a_bodies = new PhysicsBody[count];

                for (int i = 0; i < count; ++i)
                {
                    a_bodies[i] = PhysicsBody.GetBody(result[i]);
                }

                return true;
            }

            return false;
        }
        public static bool AABBCollsion(Vector3 a_min, Vector3 a_max, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = AABBCollisionS(a_min, a_max);
            if (result != null)
            {
                int count = result.Length;

                a_bodies = new PhysicsBody[count];

                for (int i = 0; i < count; ++i)
                {
                    a_bodies[i] = PhysicsBody.GetBody(result[i]);
                }

                return true;
            }

            return false;
        }
    }
}