// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#include "EnginePhysicsInterop.h"
#include "EnginePhysicsInteropStructures.h"
#include "InteropBinding.h"

ENGINE_PHYSICS_EXPORT_TABLE(IOP_BIND_FUNCTION);

namespace IcarianEngine.Physics
{
    public struct RaycastResult
    {
        /// <summary>
        /// The fraction along the ray
        /// </summary>
        public float Fraction;
        /// <summary>
        /// The position of the hit
        /// </summary>
        public Vector3 Position;
        /// <summary>
        /// The normal of the hit
        /// </summary>
        public Vector3 Normal;
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
        /// Checks if 2 ObjectLayers can collide
        /// </summary>
        /// <param name="a_layerA">The LHS layer. 0-8 range</param>
        /// <param name="a_layerB">The RHS layer. 0-8 range</param>
        /// <returns>If the layers can collide</returns>
        public static bool CanObjectLayersCollide(uint a_layerA, uint a_layerB)
        {
            return PhysicsInterop.GetObjectLayerCollision(a_layerA, a_layerB) != 0;
        }
        /// <summary>
        /// Sets if 2 ObjectLayers can collide
        /// </summary>
        /// <param name="a_layerA">The LHS layer. 0-8 range</param>
        /// <param name="a_layerB">The RHS layer. 0-8 range</param>
        /// <param name="a_state">If the layers can collide or not</param>
        public static void SetObjectLayerCollision(uint a_layerA, uint a_layerB, bool a_state)
        {
            if (a_state)
            {
                PhysicsInterop.SetObjectLayerCollision(a_layerA, a_layerB, 1);
            }
            else
            {
                PhysicsInterop.SetObjectLayerCollision(a_layerA, a_layerB, 0);
            }
        }

        /// <summary>
        /// Does a raycast in the physics simulation
        /// </summary>
        /// <param name="a_pos">The starting postion of the ray cast</param>
        /// <param name="a_dir">The direction of the ray cast</param>
        /// <param name="a_distance">The distance the ray goes</param>
        /// <param name="a_hits">Information about the hit <see cref="IcarianEngine.Physics.PhysicsBody" />. Null on no <see cref="IcarianEngine.Physics.PhysicsBody" /> hit</param>
        /// <returns>If the ray hit anything</returns>
        public static bool Raycast(Vector3 a_pos, Vector3 a_dir, float a_distance, out RaycastResult[] a_hits)
        {
            a_hits = null;

            RaycastResultBuffer[] result = PhysicsInterop.Raycast(a_pos, a_dir * a_distance);
            if (result != null)
            {
                int count = result.Length;

                List<RaycastResult> hits = new List<RaycastResult>(count);

                foreach (RaycastResultBuffer buff in result)
                {
                    PhysicsBody b = PhysicsBody.GetBody(buff.BodyAddr);
                    if (b == null)
                    {
                        continue;
                    }

                    hits.Add(new RaycastResult()
                    {
                        Fraction = buff.Fraction,
                        Position = buff.Position,
                        Normal = buff.Normal,
                        Body = b
                    });
                }

                a_hits = hits.ToArray();

                return true;
            }

            return false;
        }
        /// <summary>
        /// Does a nearest to farthest sorted raycast in the physics simulation
        /// </summary>
        /// <param name="a_pos">The starting postion of the ray cast</param>
        /// <param name="a_dir">The direction of the ray cast</param>
        /// <param name="a_distance">The distance the ray goes</param>
        /// <param name="a_hits">Information about the hit <see cref="IcarianEngine.Physics.PhysicsBody" />. Null on no <see cref="IcarianEngine.Physics.PhysicsBody" /> hit</param>
        /// <returns>If the ray hit anything</returns>
        public static bool RaycastSorted(Vector3 a_pos, Vector3 a_dir, float a_distance, out RaycastResult[] a_hits)
        {
            a_hits = null;

            RaycastResult[] results;
            if (Raycast(a_pos, a_dir, a_distance, out results))
            {
                uint count = (uint)results.LongLength;
                a_hits = new RaycastResult[count];

                for (uint i = 0; i < count; ++i)
                {
                    float frac = results[i].Fraction;

                    for (uint j = 0; j < i; ++j)
                    {
                        if (frac < a_hits[j].Fraction)
                        {
                            for (uint k = i; k > j; --k)
                            {
                                a_hits[k] = a_hits[k - 1];
                            }

                            a_hits[j] = results[i];

                            goto NextIter;
                        }
                    }

                    a_hits[i] = results[i];

                    NextIter:;
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
        /// <param name="a_bodies">The <see cref="IcarianEngine.Physics.PhysicsBody" /> hit by the sphere. Null on no <see cref="IcarianEngine.Physics.PhysicsBody" /> hit</param>
        /// <returns>If the sphere hit anything</returns>
        public static bool SphereCollision(Vector3 a_pos, float a_radius, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = PhysicsInterop.SphereCollision(a_pos, a_radius);
            if (result != null)
            {
                int count = result.Length;

                List<PhysicsBody> bodies = new List<PhysicsBody>(count);
                foreach (uint r in result)
                {
                    PhysicsBody b = PhysicsBody.GetBody(r);

                    // Can be null as it is async and can be in the process of rebuilding a body
                    // The collision functions can get funky because of it
                    // Culling them here for more sane user output
                    if (b == null)
                    {
                        continue;
                    }

                    bodies.Add(b);
                }

                a_bodies = bodies.ToArray();

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
        /// <param name="a_bodies">The <see cref="IcarianEngine.Physics.PhysicsBody" /> hit by the box. Null on no <see cref="IcarianEngine.Physics.PhysicsBody" /> hit</param>
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
        /// <param name="a_bodies">The <see cref="IcarianEngine.Physics.PhysicsBody" /> hit by the box. Null on no <see cref="IcarianEngine.Physics.PhysicsBody" /> hit</param>
        /// <returns>If the box hit anything</returns>
        public static bool BoxCollision(Matrix4 a_transform, Vector3 a_extents, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = PhysicsInterop.BoxCollision(a_transform.ToArray(), a_extents);
            if (result != null)
            {
                int count = result.Length;

                List<PhysicsBody> bodies = new List<PhysicsBody>(count);
                foreach (uint r in result)
                {
                    PhysicsBody b = PhysicsBody.GetBody(r);

                    // Can be null as it is async and can be in the process of rebuilding a body
                    // The collision functions can get funky because of it
                    // Culling them here for more sane user output
                    if (b == null)
                    {
                        continue;
                    }

                    bodies.Add(b);
                }

                a_bodies = bodies.ToArray();

                return true;
            }

            return false;
        }
        /// <summary>
        /// Does a AABB collision in the physics simulation
        /// </summary>
        /// <param name="a_min">The position of the minimum point of the AABB</param>
        /// <param name="a_max">The postion of the maximum point of the AABB</param>
        /// <param name="a_bodies">The <see cref="IcarianEngine.Physics.PhysicsBody" /> hit by the AABB. Null on no <see cref="IcarianEngine.Physics.PhysicsBody" /> hit</param>
        /// <returns>If the AABB hit anything</returns>
        public static bool AABBCollsion(Vector3 a_min, Vector3 a_max, out PhysicsBody[] a_bodies)
        {
            a_bodies = null;

            uint[] result = PhysicsInterop.AABBCollision(a_min, a_max);
            if (result != null)
            {
                int count = result.Length;

                List<PhysicsBody> bodies = new List<PhysicsBody>(count);
                foreach (uint r in result)
                {
                    PhysicsBody b = PhysicsBody.GetBody(r);

                    // Can be null as it is async and can be in the process of rebuilding a body
                    // The collision functions can get funky because of it
                    // Culling them here for more sane user output
                    if (b == null)
                    {
                        continue;
                    }

                    bodies.Add(b);
                }

                a_bodies = bodies.ToArray();

                return true;
            }

            return false;
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