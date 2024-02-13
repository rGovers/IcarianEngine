using IcarianEngine.Maths;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EnginePhysicsInterop.h"
#include "EnginePhysicsInteropStructures.h"
#include "InteropBinding.h"

ENGINE_PHYSICS_EXPORT_TABLE(IOP_BIND_FUNCTION)

namespace IcarianEngine.Physics
{
    public struct RaycastResult
    {
        /// <summary>
        /// The position of the hit
        /// </summary>
        public Vector3 Position;
        /// <summary>
        /// The body that was hit
        /// </summary>
        public PhysicsBody Body;
    };

    public static class Physics
    {
        /// <summary>
        /// The gravity of the physics simulation
        /// </summary>
        public static Vector3 Gravity
        {
            get
            {
                return PhysicsInterop.GetGravity();
            }
            set
            {
                PhysicsInterop.SetGravity(value);
            }
        }

        /// <summary>
        /// Does a raycast in the physics simulation
        /// </summary>
        /// <param name="a_pos">The starting postion of the ray cast</param>
        /// <param name="a_dir">The direction of the ray cast</param>
        /// <param name="a_distance">The distance the ray goes</param>
        /// <param name="a_hits">Information about the hit bodies</param>
        /// <returns>If the ray hit anything</returns>
        public static bool Raycast(Vector3 a_pos, Vector3 a_dir, float a_distance, out RaycastResult[] a_hits)
        {
            a_hits = null;

            RaycastResultBuffer[] result = PhysicsInterop.Raycast(a_pos, a_dir * a_distance);
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
        /// <summary>
        /// Does a sphere collision in the physics simulation
        /// </summary>
        /// <param name="a_pos">The position of the sphere</param>
        /// <param name="a_radius">The radius of the sphere</param>
        /// <param name="a_bodies">The bodies hit by the sphere</param>
        /// <returns>If the sphere hit anything</returns>
        public static bool SphereCollision(Vector3 a_pos, float a_radius, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = PhysicsInterop.SphereCollision(a_pos, a_radius);
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

        /// <summary>
        /// Does a box collision in the physics simulation
        /// </summary>
        /// <param name="a_pos">The position of the box</param>
        /// <param name="a_rotation">The rotation of the box</param>
        /// <param name="a_extents">The extents of the box</param>
        /// <param name="a_bodies">The bodies hit by the box</param>
        /// <returns>If the box hit anything</returns>
        public static bool BoxCollision(Vector3 a_pos, Quaternion a_rotation, Vector3 a_extents, out PhysicsBody[] a_bodies)
        {
            Matrix4 mat = Matrix4.FromTransform(a_pos, a_rotation, Vector3.One);

            return BoxCollision(mat, a_extents, out a_bodies);
        }
        /// <summary>
        /// Does a box collision in the physics simulation
        /// </summary>
        /// <param name="a_transform">The transform matrix of the box</param>
        /// <param name="a_extents">The extents of the box</param>
        /// <param name="a_bodies">The bodies hit by the box</param>
        /// <returns>If the box hit anything</returns>
        public static bool BoxCollision(Matrix4 a_transform, Vector3 a_extents, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = PhysicsInterop.BoxCollision(a_transform.ToArray(), a_extents);
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
        /// <summary>
        /// Does a AABB collision in the physics simulation
        /// </summary>
        /// <param name="a_min">The position of the minimum point of the AABB</param>
        /// <param name="a_max">The postion of the maximum point of the AABB</param>
        /// <param name="a_bodies">The bodies hit by the AABB</param>
        /// <returns>If the AABB hit anything</returns>
        public static bool AABBCollsion(Vector3 a_min, Vector3 a_max, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = PhysicsInterop.AABBCollision(a_min, a_max);
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