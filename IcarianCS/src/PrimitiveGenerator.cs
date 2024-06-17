using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using System;
using System.Collections.Generic;

// https://en.wikipedia.org/wiki/Pairing_function#Cantor_pairing_function
// C# is funny about inlining and not immediately obvious what it does so define instead of function
#define PAIRING(A, B) (((ulong)A + B) * ((ulong)A + B + 1) / 2 + B)

namespace IcarianEngine
{
    /// <summary>
    /// Primitive Type enumeration
    /// </summary>
    public enum PrimitiveType
    {
        Cube,
        IcoSphere,
        Torus,
        Cylinder
    };

    public static class PrimitiveGenerator
    {
        /// <summary>
        /// Creates a Cube primitive <see cref="IcarianEngine.Rendering.Model"/>
        /// </summary>
        /// <returns>The created <see cref="IcarianEngine.Rendering.Model"/></returns>
        public static Model CreateCube()
        {
            Vertex[] vertices = 
            {
                // 0
                new Vertex() 
                {
                    Position = new Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(-1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, -1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, -1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero

                },

                // 3
                new Vertex() 
                {
                    Position = new Vector4(1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, -1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, -1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 6
                new Vertex() 
                {
                    Position = new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(-1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, -1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, 1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 9
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, -1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, 1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 12
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(-1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, -1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 15
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, -1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 18
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(-1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, 1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 21
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(1.0f, 0.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 1.0f, 0.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = new Vector3(0.0f, 0.0f, 1.0f),
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                }
            };
            uint[] indices = 
            {
                0,  6,  18, 0,  18, 12,
                1,  4,  10, 1,  10, 7,
                2,  17, 5,  2,  14, 17,
                3,  21, 9,  3,  15, 21,
                8,  11, 23, 8,  23, 20,
                13, 22, 16, 13, 19, 22
            };

            return Model.CreateModel(vertices, indices, Vector3.One.Magnitude);
        }   

        static uint PlaceIcoVertex(uint a_indexA, uint a_indexB, ref uint[] a_map, ref List<Vector3> a_vertices)
        {
            ulong key = PAIRING(a_indexA, a_indexB);
            if (a_map[key] != uint.MaxValue)
            {
                return a_map[key];
            }

            Vector3 vA = a_vertices[(int)a_indexA];
            Vector3 vB = a_vertices[(int)a_indexB];

            uint index = (uint)a_vertices.Count;
            a_map[key] = index;

            a_vertices.Add(Vector3.Normalized(vA + vB));

            return index;
        }

        /// <summary>
        /// Creates a IcoSphere primitive <see cref="IcarianEngine.Rendering.Model" />
        /// </summary>
        /// <param name="a_subDivisions">How many times to sub divide the Ico Sphere</param>
        /// <returns>The created <see cref="IcarianEngine.Rendering.Model"/></returns>
        public static Model CreateIcoSphere(uint a_subDivisions)
        {
            const float X = 0.525731112119133606f;
            const float Z = 0.850650808352039932f;
            const float N = 0.0f;

            List<Vector3> tempVerts = new List<Vector3>(new Vector3[]
            {
                new Vector3(-X, N, Z), new Vector3(X, N, Z),  new Vector3(-X, N, -Z), new Vector3(X, N, -Z),
                new Vector3(N, Z, X),  new Vector3(N, Z, -X), new Vector3(N, -Z, X),  new Vector3(N, -Z, -X),
                new Vector3(Z, X, N),  new Vector3(-Z, X, N), new Vector3(Z, -X, N),  new Vector3(-Z, -X, N)
            });
            List<uint> indices = new List<uint>(new uint[]
            {
                0, 1, 4,    0, 4, 9,    9, 4, 5,    4, 8, 5,    4, 1, 8,
                8, 1, 10,   8, 10, 3,   5, 8, 3,    5, 3, 2,    2, 3, 7,
                7, 3, 10,   7, 10, 6,   7, 6, 11,   11, 6, 0,   0, 6, 1,
                6, 10, 1,   9, 11, 0,   9, 2, 11,   9, 5, 2,    7, 11, 2
            });

            for (uint i = 0; i < a_subDivisions; ++i)
            {
                uint indexCount = (uint)indices.Count;
                ulong mapSize = (ulong)indexCount * indexCount;
                
                // Golden rule of use vector by default
                // Use a pairing function to index into instead of using a Dictionary
                uint[] map = new uint[mapSize];
                // C# Std Lib gets access to build flags that I do not therefore faster then a for loop despite a for loop being the implementation
                Array.Fill(map, uint.MaxValue);

                uint triCount = indexCount / 3;

                List<uint> newIndices = new List<uint>();

                for (uint j = 0; j < triCount; ++j)
                {
                    uint indexA = indices[(int)(j * 3 + 0)];
                    uint indexB = indices[(int)(j * 3 + 1)];
                    uint indexC = indices[(int)(j * 3 + 2)];

                    uint midA = PlaceIcoVertex(indexA, indexB, ref map, ref tempVerts);
                    uint midB = PlaceIcoVertex(indexB, indexC, ref map, ref tempVerts);
                    uint midC = PlaceIcoVertex(indexC, indexA, ref map, ref tempVerts);

                    newIndices.Add(indexA); newIndices.Add(midA); newIndices.Add(midC);
                    newIndices.Add(indexB); newIndices.Add(midB); newIndices.Add(midA);
                    newIndices.Add(indexC); newIndices.Add(midC); newIndices.Add(midB);
                    newIndices.Add(midA);   newIndices.Add(midB); newIndices.Add(midC);
                }

                indices = newIndices;
            }

            uint vertexCount = (uint)tempVerts.Count;
            Vertex[] verts = new Vertex[vertexCount];

            for (uint i = 0; i < vertexCount; ++i)
            {
                Vector3 point = tempVerts[(int)i];

                verts[i] = new Vertex()
                {
                    Position = new Vector4(point, 1.0f),
                    Normal = point,
                    Color = Vector4.One,
                    TexCoords = Vector2.One
                };
            }

            return Model.CreateModel(verts, indices.ToArray(), 1.0f);
        }

        /// <summary>
        /// Create a Torus primitive <see cref="IcarianEngine.Rendering.Model" />
        /// </summary>
        /// <param name="a_loopRadius">The radius of each loop around the perimeter of the torus</param>
        /// <param name="a_loopSegments">The number of segments in each loop of the torus</param>
        /// <param name="a_ringRadius">The radius of the perimeter ring of the torus</param>
        /// <param name="a_ringSegments">The number of segments around the perimeter ring of the torus</param>
        /// <returns>The created <see cref="IcarianEngine.Rendering.Model" /></returns>
        public static Model CreateTorus(float a_loopRadius, uint a_loopSegments, float a_ringRadius, uint a_ringSegments)
        {
            Vector2[] loopVertices = new Vector2[a_loopSegments];
            Vector2[] loopNormals = new Vector2[a_loopSegments];

            for (uint i = 0; i < a_loopSegments; ++i)
            {
                float rot = (float)i / a_loopSegments * Mathf.TwoPI;

                loopNormals[i] = new Vector2(Mathf.Sin(rot), Mathf.Cos(rot));
                loopVertices[i] = loopNormals[i] * a_loopRadius;
            }

            uint loopIndicies = a_loopSegments * 6;

            Vertex[] vertices = new Vertex[a_loopSegments * a_ringSegments];
            uint[] indices = new uint[loopIndicies * a_ringSegments];

            Vector3 ringOffset = Vector3.UnitZ * a_ringRadius;

            for (uint i = 0; i < a_ringSegments; ++i)
            {
                Quaternion rot = Quaternion.FromAxisAngle(Vector3.UnitY, (float)i / a_ringSegments * Mathf.TwoPI);

                uint vertexOffset = ((i + 0) % a_ringSegments) * a_loopSegments;
                uint nextVertexOffset = ((i + 1) % a_ringSegments) * a_loopSegments;

                uint indicesOffset = i * loopIndicies;

                for (uint j = 0; j < a_loopSegments; ++j)
                {
                    Vertex v;

                    v.Position = new Vector4(rot * (ringOffset + new Vector3(0.0f, loopVertices[j].Y, loopVertices[j].X)), 1);
                    v.Normal = rot * new Vector3(0.0f, loopNormals[j].Y, loopNormals[j].X);
                    v.Color = Vector4.One;
                    v.TexCoords = Vector2.Zero;

                    vertices[vertexOffset + j] = v; 

                    uint indexA = vertexOffset + (j + 0) % a_loopSegments;
                    uint indexB = nextVertexOffset + (j + 0) % a_loopSegments;
                    uint indexC = nextVertexOffset + (j + 1) % a_loopSegments;
                    uint indexD = vertexOffset + (j + 1) % a_loopSegments;

                    uint finalIndexOffset = indicesOffset + j * 6;

                    indices[finalIndexOffset + 0] = indexA;
                    indices[finalIndexOffset + 1] = indexD;
                    indices[finalIndexOffset + 2] = indexB;

                    indices[finalIndexOffset + 3] = indexB;
                    indices[finalIndexOffset + 4] = indexD;
                    indices[finalIndexOffset + 5] = indexC; 
                }
            }

            return Model.CreateModel(vertices, indices, 1.0f + a_loopRadius);
        }

        /// <summary>
        /// Create a Cylinder primitive <see cref="IcarianEngine.Rendering.Model" />
        /// </summary>
        /// <param name="a_radius">The radius of the loops</param>
        /// <param name="a_height">The height of the cylinder</param>
        /// <param name="a_segments">The number of segements in the loop of the cylinder</param>
        /// <returns>The created <see cref="IcarianEngine.Rendering.Model" /></returns>
        public static Model CreateCylinder(float a_radius, float a_height, uint a_segments)
        {
            // Naive and can be improved but if people are creating a lot of primitives we have bigger problems performance wise
            // If performance was a concern would be doing it on the C++ side anyway because of unpacking
            uint loopOffset = a_segments + 1;
            uint loopEnd = loopOffset * 2;
            uint vertexCount = loopEnd + (a_segments * 2);
            Vector3[] positions = new Vector3[vertexCount];
            Vector3[] normals = new Vector3[vertexCount];

            float halfHeight = a_height * 0.5f;

            positions[0] = Vector3.UnitY * halfHeight;
            positions[loopOffset] = -Vector3.UnitY * halfHeight;
            Array.Fill(normals, Vector3.UnitY, 0, (int)loopOffset);
            Array.Fill(normals, -Vector3.UnitY, (int)loopOffset, (int)loopOffset);

            for (uint i = 0; i < a_segments; ++i)
            {
                float angle = (float)i / a_segments * Mathf.TwoPI;
                Vector2 dir = new Vector2(Mathf.Sin(angle), Mathf.Cos(angle));

                Vector3 norm = new Vector3(dir.X, 0.0f, dir.Y);
                Vector2 sDir = dir * a_radius;
                Vector3 posA = new Vector3(sDir.X, halfHeight,  sDir.Y);
                Vector3 posB = new Vector3(sDir.X, -halfHeight, sDir.Y);

                uint vertexOffset = i + 1;
                uint indexA = vertexOffset;
                uint indexB = vertexOffset + loopOffset;
                uint indexC = loopEnd + i;
                uint indexD = loopEnd + a_segments + i;

                positions[indexA] = posA;
                positions[indexB] = posB;
                positions[indexC] = posA;
                positions[indexD] = posB;
                
                normals[indexC] = norm;
                normals[indexD] = norm;
            }

            uint indexCount = a_segments * 12;
            uint[] indices = new uint[indexCount];

            for (uint i = 0; i < a_segments; ++i)
            {
                uint tIndex = i * 3;
                uint bIndex = i * 3 + a_segments * 3;
                uint qIndex = i * 6 + a_segments * 6;

                indices[tIndex + 0] = i + 1;
                indices[tIndex + 1] = (i + 1) % a_segments + 1;
                indices[tIndex + 2] = 0;

                indices[bIndex + 0] = ((i + 1) % a_segments) + 1 + loopOffset;
                indices[bIndex + 1] = i + 1 + loopOffset;
                indices[bIndex + 2] = loopOffset;

                uint quadIndexA = i + loopEnd;
                uint quadIndexB = (i + 1) % a_segments + loopEnd;
                uint quadIndexC = i + loopEnd + a_segments;
                uint quadIndexD = (i + 1) % a_segments + loopEnd + a_segments;

                indices[qIndex + 0] = quadIndexA;
                indices[qIndex + 1] = quadIndexC;
                indices[qIndex + 2] = quadIndexB;
                
                indices[qIndex + 3] = quadIndexB;
                indices[qIndex + 4] = quadIndexC;
                indices[qIndex + 5] = quadIndexD;
            }

            Vertex[] vertices = new Vertex[vertexCount];

            for (uint i = 0; i < vertexCount; ++i)
            {
                vertices[i] = new Vertex()
                {
                    Position = new Vector4(positions[i], 1.0f),
                    Normal = normals[i],
                    Color = Vector4.One,
                    TexCoords = Vector2.One
                };
            }

            return Model.CreateModel(vertices, indices, new Vector2(halfHeight, a_radius).Magnitude);
        }

        /// <summary>
        /// Creates a primitive <see cref="IcarianEngine.Rendering.Model" />
        /// </summary>
        /// <param name="a_primitiveType">The type of primitive to create</param>
        /// <returns>The type of <see cref="IcarianEngine.Rendering.Model"/> specified</returns>
        public static Model CreatePrimitive(PrimitiveType a_primitiveType)
        {
            switch (a_primitiveType)
            {
            case PrimitiveType.Cube:
            {
                return CreateCube();
            }
            case PrimitiveType.IcoSphere:
            {
                return CreateIcoSphere(1);
            }
            case PrimitiveType.Torus:
            {
                return CreateTorus(0.25f, 8, 1.0f, 16);
            }
            case PrimitiveType.Cylinder:
            {
                return CreateCylinder(0.25f, 1.0f, 16);
            }
            }

            return null;   
        }
    }
}