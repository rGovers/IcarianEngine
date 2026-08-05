// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "AI/NavigationMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/norm.hpp>

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "IO.h"
#include "Trace.h"

NavigationMesh::NavigationMesh(const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator, IcarianCore::Allocator* a_tempAllocator)
{
    TRACE("Creating Nav Mesh");
    m_allocator = a_allocator;

    m_vertexCount = 0;
    m_vertices = nullptr;
    m_faceCount = 0;
    m_faces = nullptr;

    const IcarianCore::COWU8String ext = IO::GetExtension(a_path, a_tempAllocator);

    switch (StringHash<uint32_t>(ext.CStr()))
    {
    case StringHash<uint32_t>(".obj"):
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        IVERIFY(handle != nullptr);
        IDEFER(IcarianCore::MallocAllocator::Instance->Destroy(handle));

        const uint64_t size = handle->GetSize();
        uint8_t* dat = a_tempAllocator->TAllocate<uint8_t>(size);
        IDEFER(a_tempAllocator->Free(dat));
        if (handle->Read(dat, size) != size)
        {
            IERROR("Failed reading mesh data: " + a_path);

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory
        (
            dat,
            (size_t)size,
            aiProcess_Triangulate | aiProcess_PreTransformVertices,
            ext.CStr() + 1
        );
        IVERIFY(scene != nullptr);
        if (scene->mNumMeshes <= 0)
        {
            break;
        }

        struct EdgeTable
        {
            uint32_t Index[2];
            uint32_t Edge[2];
        };

        IcarianCore::Dictionary<uint64_t, EdgeTable> edgeMap = IcarianCore::Dictionary<uint64_t, EdgeTable>(a_tempAllocator);

        const aiMesh* mesh = scene->mMeshes[0];

        const uint32_t vertexCount = (uint32_t)mesh->mNumVertices;

        glm::vec3* vertices = a_tempAllocator->ZTAllocate<glm::vec3>(vertexCount);
        IDEFER(a_tempAllocator->Free(vertices));

        uint32_t* vertexMap = a_tempAllocator->ZTAllocate<uint32_t>(vertexCount);
        IDEFER(a_tempAllocator->Free(vertexMap));
        memset(vertexMap, -1, vertexCount * sizeof(uint32_t));

        const uint32_t faceCount = (uint32_t)mesh->mNumFaces;

        NavigationFace* faces = a_tempAllocator->ZTAllocate<NavigationFace>(faceCount);
        IDEFER(a_tempAllocator->Free(faces));

        // First pass load in the model data and cull faces pointing down as they will not be navigable so no point having them in the nav mesh
        // Cannot guarantee normals so we calculate them ourselves
        for (uint32_t i = 0; i < faceCount; ++i)
        {
            const aiFace& face = mesh->mFaces[i];

            glm::vec3 positions[3];
            for (uint32_t j = 0; j < 3; ++j)
            {
                const uint32_t index = face.mIndices[j];

                const aiVector3D& pos = mesh->mVertices[index];
                positions[j] = glm::vec3(pos.x, -pos.y, pos.z);
            }

            const glm::vec3 dirA = positions[0] - positions[1];
            const glm::vec3 dirB = positions[0] - positions[2];
            const glm::vec3 norm = glm::cross(dirB, dirA);

            const float dot = glm::dot(norm, glm::vec3(0.0f, -1.0f, 0.0f));
            if (dot <= 0.0f)
            {
                continue;
            }

            const glm::vec3 center = (positions[0] + positions[1] + positions[2]) * 0.33f;
            NavigationFace navFace =
            {
                .Connections = { uint32_t(-1), uint32_t(-1), uint32_t(-1) },
                .Center = center,
                .Dot = dot
            };

            for (uint32_t j = 0; j < 3; ++j)
            {
                const uint32_t index = face.mIndices[j];
                const uint32_t vMapIndex = vertexMap[index];
                if (vMapIndex != uint32_t(-1))
                {
                    navFace.Indicies[j] = vMapIndex;

                    continue;
                }

                navFace.Indicies[j] = m_vertexCount;
                vertexMap[index] = m_vertexCount;
                vertices[m_vertexCount++] = positions[j];
            }

            for (uint32_t j = 0; j < 3; ++j)
            {
                const uint32_t index = navFace.Indicies[j];
                const uint32_t nextIndex = navFace.Indicies[(j + 1) % 3];

                const uint32_t indexA = ILAMBDA(
                {
                    if (index < nextIndex)
                    {
                        ILRETURN index;
                    }

                    ILRETURN nextIndex;
                });
                const uint32_t indexB = ILAMBDA(
                {
                    if (index < nextIndex)
                    {
                        ILRETURN nextIndex;
                    }

                    ILRETURN index;
                });

                const uint64_t key = (uint64_t)indexA << 31 | (uint64_t)indexB;
                if (edgeMap.Exists(key))
                {
                    EdgeTable& val = edgeMap[key];
                    val.Index[1] = m_faceCount;
                    val.Edge[1] = j;

                    continue;
                }

                const EdgeTable table =
                {
                    .Index = { m_faceCount, uint32_t(-1) },
                    .Edge = { j, uint32_t(-1) },
                };

                edgeMap.Push(key, table);
            }

            faces[m_faceCount++] = navFace;
        }

        const IcarianCore::Array<EdgeTable> edges = edgeMap.GetValues(a_tempAllocator);
        // Second pass we want to link all the face connections
        for (const EdgeTable& table : edges)
        {
            const uint32_t indexA = table.Index[0];
            if (indexA == uint32_t(-1))
            {
                continue;
            }

            const uint32_t indexB = table.Index[1];
            if (indexB == uint32_t(-1))
            {
                continue;
            }

            faces[indexA].Connections[table.Edge[0]] = indexB;
            faces[indexB].Connections[table.Edge[1]] = indexA;
        }

        m_vertices = m_allocator->TAllocate<glm::vec3>(m_vertexCount);
        for (uint32_t i = 0; i < m_vertexCount; ++i)
        {
            m_vertices[i] = vertices[i];
        }

        m_faces = m_allocator->TAllocate<NavigationFace>(m_faceCount);
        for (uint32_t i = 0; i < m_faceCount; ++i)
        {
            m_faces[i] = faces[i];
        }

        break;
    }
    default:
    {
        IERROR("Invalid model file extension: " + a_path);

        break;
    }
    }
}
NavigationMesh::~NavigationMesh()
{
    if (m_vertices != nullptr)
    {
        m_allocator->Free(m_vertices);
    }

    if (m_faces != nullptr)
    {
        m_allocator->Free(m_faces);
    }
}

uint32_t NavigationMesh::GetIndex(const glm::vec3& a_point) const
{
    float dist = std::numeric_limits<float>::max();
    uint32_t triangle = -1;
    // Do not want the first tri we collide with incase have 2 tris stacked
    // May change to use a spatial hash to narrow down the line if we have large meshes
    for (uint32_t i = 0; i < m_faceCount; ++i)
    {
        const NavigationFace& face = m_faces[i];

        const glm::vec3& vertA = m_vertices[face.Indicies[0]];
        const glm::vec3& vertB = m_vertices[face.Indicies[1]];
        const glm::vec3& vertC = m_vertices[face.Indicies[2]];

        const float orig = glm::abs((vertB.x - vertA.x) * (vertC.z - vertA.z) - (vertC.x - vertA.x) * (vertB.z - vertA.z));

        const float a1 = glm::abs((vertA.x - a_point.x) * (vertB.z - a_point.z) - (vertB.x - a_point.x) * (vertA.z - a_point.z));
        const float a2 = glm::abs((vertB.x - a_point.x) * (vertC.z - a_point.z) - (vertC.x - a_point.x) * (vertB.z - a_point.z));
        const float a3 = glm::abs((vertC.x - a_point.x) * (vertA.z - a_point.z) - (vertA.x - a_point.x) * (vertC.z - a_point.z));

        // Do not trust floating point values
        if (glm::abs((a1 + a2 + a3) - orig) >= 0.001f)
        {
            continue;
        }

        const float mag = a_point.y - face.Center.y;
        if (mag >= dist)
        {
            continue;
        }

        triangle = i;
        dist = mag;
    }

    return triangle;
}

struct PathNode
{
    float Weight;
    uint32_t Index;
};

static void PushPathValue
(
    uint32_t a_index,
    const NavigationFace* a_faces,
    const glm::vec3& a_end,
    IcarianCore::Array<PathNode>* a_queue,
    IcarianCore::Dictionary<uint32_t, uint32_t>* a_stepMap
)
{
    const NavigationFace& face = a_faces[a_index];

    for (uint32_t i = 0; i < 3; ++i)
    {
        const uint32_t con = face.Connections[i];
        if (con == uint32_t(-1))
        {
            continue;
        }

        if (a_stepMap->Exists(con))
        {
            continue;
        }

        a_stepMap->Push(con, a_index);

        const NavigationFace& conFace = a_faces[con];

        const glm::vec3 diff = a_end - conFace.Center;
        const float d = glm::length2(diff);

        const PathNode value =
        {
            .Weight = d,
            .Index = con
        };

        const uint32_t queueSize = a_queue->Size();
        for (int32_t j = queueSize - 1; j >= 0; --j)
        {
            const PathNode& node = (*a_queue)[j];
            if (d >= node.Weight)
            {
                continue;
            }

            a_queue->Insert(j, value);

            goto NextIter;
        }

        a_queue->Push(value);

NextIter:;
    }
}

static float TriToAreaSqr(const glm::vec3& a_vertA, const glm::vec3& a_vertB, const glm::vec3& a_vertC)
{
    const glm::vec2 base = a_vertA.xz();

    const glm::vec2 a = a_vertB.xz() - base;
    const glm::vec2 b = a_vertC.xz() - base;

    return b.x * a.y - a.x * b.y;
}

IcarianCore::Array<glm::vec3> NavigationMesh::GeneratePath
(
    const glm::vec3& a_startPoint,
    const glm::vec3& a_endPoint,
    float a_agentRadius,
    IcarianCore::Allocator* a_allocator,
    IcarianCore::Allocator* a_tempAllocator
) const
{
    const uint32_t indexA = GetIndex(a_startPoint);
    if (indexA == uint32_t(-1))
    {
        return IcarianCore::Array<glm::vec3>(a_allocator);
    }

    const uint32_t indexB = GetIndex(a_endPoint);
    if (indexB == uint32_t(-1))
    {
        return IcarianCore::Array<glm::vec3>(a_allocator);
    }

    return GeneratePath(a_startPoint, a_endPoint, indexA, indexB, a_agentRadius, a_allocator, a_tempAllocator);
}
// 2.5D Pathfinding
IcarianCore::Array<glm::vec3> NavigationMesh::GeneratePath
(
    const glm::vec3& a_startPoint,
    const glm::vec3& a_endPoint,
    uint32_t a_startIndex,
    uint32_t a_endIndex,
    float a_agentRadius,
    IcarianCore::Allocator* a_allocator,
    IcarianCore::Allocator* a_tempAllocator
) const
{
    if (a_startIndex == uint32_t(-1) || a_endIndex == uint32_t(-1))
    {
        return IcarianCore::Array<glm::vec3>(a_allocator);
    }

    if (a_startIndex == a_endIndex)
    {
        IcarianCore::Array<glm::vec3> path = IcarianCore::Array<glm::vec3>(a_allocator);

        path.Push(a_startPoint);
        path.Push(a_endPoint);

        return path;
    }

    // Find path
    IcarianCore::Array<PathNode> queue = IcarianCore::Array<PathNode>(a_tempAllocator);
    IcarianCore::Dictionary<uint32_t, uint32_t> stepMap = IcarianCore::Dictionary<uint32_t, uint32_t>(a_tempAllocator);
    PushPathValue(a_startIndex, m_faces, a_endPoint, &queue, &stepMap);
    while (!queue.Empty())
    {
        const PathNode value = queue.Get(0);
        queue.Erase(0);
        if (value.Index == a_endIndex)
        {
            break;
        }

        PushPathValue(value.Index, m_faces, a_endPoint, &queue, &stepMap);
    }

    // Backtrace path
    IcarianCore::Array<uint32_t> pathIndices = IcarianCore::Array<uint32_t>(a_tempAllocator);

    uint32_t node = a_endIndex;
    pathIndices.Push(node);
    while (node != a_startIndex)
    {
        const uint32_t index = stepMap[node];
        IDEFER(node = index);

        pathIndices.Insert(0, index);
    }
    pathIndices.Insert(0, a_startIndex);

    struct Portal
    {
        uint32_t LeftIndex;
        uint32_t RightIndex;
    };

    // Build portals
    // TODO: Adjust portals based off agent radius and take agent radius as a parameter
    // NOTE: While should build meshes based off the agent gets messy when dealing with agent of varying size as can have several meshes and alot of "wasted" memory
    // In reality building the mesh based off the biggest agent and adjusting portals should be fine outside of extreme size differences
    const uint32_t pathIndexCount = pathIndices.Size();

    IcarianCore::Array<Portal> portals = IcarianCore::Array<Portal>(a_tempAllocator);
    portals.Reserve(pathIndexCount);

    for (uint32_t i = 1; i < pathIndexCount; ++i)
    {
        const uint32_t pastIndex = pathIndices[i - 1];
        const uint32_t curIndex = pathIndices[i];

        const NavigationFace& pastFace = m_faces[pastIndex];
        const NavigationFace& curFace = m_faces[curIndex];

        for (uint32_t j = 0; j < 3; ++j)
        {
            if (pastFace.Connections[j] != curIndex)
            {
                continue;
            }

            const uint32_t indexA = pastFace.Indicies[j];
            const uint32_t indexB = pastFace.Indicies[(j + 1) % 3];

            const glm::vec2 centerA = pastFace.Center.xz();
            const glm::vec2 centerB = curFace.Center.xz();
            // Should not happen but has the potential to
            if (centerA == centerB)
            {
                break;
            }

            const glm::vec2 diff = centerB - centerA;
            const glm::vec2 right = glm::vec2(diff.y, -diff.x);

            const glm::vec2 vertPos = m_vertices[indexA].xz();
            const glm::vec2 vertDiff = vertPos - centerA;

            // Non normalized but should not matter as only after the direction of the vector
            const float dot = glm::dot(vertDiff, right);

            const Portal port =
            {
                .LeftIndex = ILAMBDA(
                {
                    if (dot > 0)
                    {
                        ILRETURN indexB;
                    }

                    ILRETURN indexA;
                }),
                .RightIndex = ILAMBDA(
                {
                    if (dot > 0)
                    {
                        ILRETURN indexA;
                    }

                    ILRETURN indexB;
                })
            };

            portals.Push(port);

            break;
        }
    }

    // Pull path tight
    const uint32_t portalCount = portals.Size();

    IcarianCore::Array<glm::vec3> path = IcarianCore::Array<glm::vec3>(a_allocator);
    path.Reserve(portalCount + 1);
    path.Push(a_startPoint);

    uint32_t portalLeftIndex = 0;
    uint32_t portalRightIndex = 0;
    uint32_t portalApexIndex = 0;
    glm::vec3 portalApex = a_startPoint;
    glm::vec3 portalLeft = a_startPoint;
    glm::vec3 portalRight = a_startPoint;

    constexpr float Epsilon = 0.001f * 0.001f;
    // Credit: http://digestingduck.blogspot.com/2010/03/simple-stupid-funnel-algorithm.html
    // Made some adjustments but pretty much the same as what is linked
    for (uint32_t i = 0; i < portalCount; ++i)
    {
        const Portal port = portals[i];

        const uint32_t leftIndex = port.LeftIndex;
        const uint32_t rightIndex = port.RightIndex;

        const glm::vec3& leftVertex = m_vertices[leftIndex];
        const glm::vec3& rightVertex = m_vertices[rightIndex];

        const glm::vec3 axis = glm::normalize(rightVertex - leftVertex);
        const glm::vec3 offset = axis * a_agentRadius;

        const glm::vec3 leftP = leftVertex + offset;
        const glm::vec3 rightP = rightVertex - offset;

        // NOTE: Technically not correct to do in 2 dimensions but should be fine as we do not need to follow the path exactly and only need waypoints
        // If we need to follow the path exactly may need to change down the line
        if (TriToAreaSqr(portalApex, portalRight, rightP) <= 0.0f)
        {
            const float eq = glm::length2(portalApex - portalRight);
            if (eq <= Epsilon || TriToAreaSqr(portalApex, portalLeft, rightP) > 0.0f)
            {
                portalRight = rightP;
                portalRightIndex = i;
            }
            else
            {
                path.Push(portalLeft);

                portalApex = portalLeft;
                portalApexIndex = portalLeftIndex;
                portalRight = portalApex;
                portalRightIndex = portalApexIndex;

                i = portalApexIndex;

                continue;
            }
        }

        if (TriToAreaSqr(portalApex, portalLeft, leftP) >= 0.0f)
        {
            const float eq = glm::length2(portalApex - portalLeft);
            if (eq <= Epsilon || TriToAreaSqr(portalApex, portalRight, leftP) < 0.0f)
            {
                portalLeft = leftP;
                portalLeftIndex = i;
            }
            else
            {
                path.Push(portalRight);

                portalApex = portalRight;
                portalApexIndex = portalRightIndex;
                portalLeft = portalApex;
                portalLeftIndex = portalApexIndex;

                i = portalApexIndex;

                continue;
            }
        }
    }

    path.Push(a_endPoint);

    return path;
}

// MIT License
//
// Copyright (c) 2026 River Govers
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
