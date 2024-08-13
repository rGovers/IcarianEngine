// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "AI/NavigationMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/norm.hpp>

#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Trace.h"

NavigationMesh::NavigationMesh(const std::filesystem::path& a_path)
{
    m_vertexCount = 0;
    m_vertices = nullptr;
    m_faceCount = 0;
    m_faces = nullptr;

    TRACE("Creating Nav Mesh");
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    switch (StringHash<uint32_t>(extStr.c_str()))
    {
    case StringHash<uint32_t>(".obj"):
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        IVERIFY(handle != nullptr);
        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        uint8_t* dat = new uint8_t[size];
        IDEFER(delete[] dat);
        if (handle->Read(dat, size) != size)
        {
            IERROR("Failed reading mesh data: " + a_path.string());

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, aiProcess_Triangulate | aiProcess_PreTransformVertices, extStr.c_str() + 1);
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

        std::unordered_map<uint64_t, EdgeTable> edgeMap;
        
        const aiMesh* mesh = scene->mMeshes[0];

        const uint32_t vertexCount = (uint32_t)mesh->mNumVertices;

        glm::vec3* vertices = new glm::vec3[vertexCount];
        IDEFER(delete[] vertices);

        uint32_t* vertexMap = new uint32_t[vertexCount];
        IDEFER(delete[] vertexMap);
        memset(vertexMap, -1, vertexCount * sizeof(uint32_t));

        const uint32_t faceCount = (uint32_t)mesh->mNumFaces;

        NavigationFace* faces = new NavigationFace[faceCount];
        IDEFER(delete[] faces);

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
                if (vMapIndex != -1)
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

                uint32_t indexA;
                uint32_t indexB;
                if (index < nextIndex)
                {
                    indexA = index;
                    indexB = nextIndex;
                }
                else
                {
                    indexA = nextIndex;
                    indexB = index;
                }

                const uint64_t key = (uint64_t)indexA << 31 | (uint64_t)indexB;
                const auto iter = edgeMap.find(key);
                if (iter != edgeMap.end())
                {
                    iter->second.Edge[1] = j;
                    iter->second.Index[1] = m_faceCount;

                    continue;
                }

                const EdgeTable table = 
                {
                    .Index = { m_faceCount, uint32_t(-1) },
                    .Edge = { j, uint32_t(-1) },
                };

                edgeMap.emplace(key, table);
            }

            faces[m_faceCount++] = navFace;
        }

        // Second pass we want to link all the face connections
        for (const auto iter : edgeMap)
        {
            const EdgeTable table = iter.second;

            const uint32_t indexA = table.Index[0];
            const uint32_t indexB = table.Index[1];

            if (indexA == -1 || indexB == -1)
            {
                continue;
            }

            faces[indexA].Connections[table.Edge[0]] = indexB;
            faces[indexB].Connections[table.Edge[1]] = indexA;
        }

        m_vertices = new glm::vec3[m_vertexCount];
        std::copy(vertices, vertices + m_vertexCount, m_vertices);
        m_faces = new NavigationFace[m_faceCount];
        std::copy(faces, faces + m_faceCount, m_faces);

        break;
    }
    default:
    {
        IERROR("Invalid model file extension: " + a_path.string());

        break;
    }
    }
}
NavigationMesh::~NavigationMesh()
{
    if (m_vertices != nullptr)
    {
        delete[] m_vertices;
    }

    if (m_faces != nullptr)
    {
        delete[] m_faces;
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
        if (glm::abs((a1 + a2 + a3) - orig) < 0.001f)
        {
            const float mag = a_point.y - face.Center.y;
            
            if (mag < dist)
            {
                triangle = i;
                dist = mag;
            }
        }
    }

    return triangle;
}

struct PathNode
{
    float Weight;
    uint32_t Index;
};

static void PushPathValue(uint32_t a_index, const NavigationFace* a_faces, const glm::vec3& a_end, Array<PathNode>* a_queue, std::unordered_map<uint32_t, uint32_t>* a_stepMap)
{
    const NavigationFace& face = a_faces[a_index];

    for (uint32_t i = 0; i < 3; ++i)
    {
        const uint32_t con = face.Connections[i];
        if (con == -1)
        {
            continue;
        }

        const auto iter = a_stepMap->find(con);
        if (iter != a_stepMap->end())
        {
            continue;
        }

        a_stepMap->emplace(con, a_index);

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
            if (d < a_queue->Get(j).Weight)
            {
                a_queue->Insert(j, value);

                goto NextIter;
            }
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

Array<glm::vec3> NavigationMesh::GeneratePath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, float a_agentRadius) const
{
    const uint32_t indexA = GetIndex(a_startPoint);
    if (indexA == -1)
    {
        return Array<glm::vec3>();
    }
    const uint32_t indexB = GetIndex(a_endPoint);
    if (indexB == -1)
    {
        return Array<glm::vec3>();
    }

    return GeneratePath(a_startPoint, a_endPoint, indexA, indexB, a_agentRadius);
}
// 2.5D Pathfinding
Array<glm::vec3> NavigationMesh::GeneratePath(const glm::vec3& a_startPoint, const glm::vec3& a_endPoint, uint32_t a_startIndex, uint32_t a_endIndex, float a_agentRadius) const
{
    if (a_startIndex == -1 || a_endIndex == -1)
    {
        return Array<glm::vec3>();
    }

    if (a_startIndex == a_endIndex)
    {
        Array<glm::vec3> path;

        path.Push(a_startPoint);
        path.Push(a_endPoint);

        return path;
    }

    // Find path
    Array<PathNode> queue;
    std::unordered_map<uint32_t, uint32_t> stepMap;
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
    Array<uint32_t> pathIndices;

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
    
    Array<Portal> portals;
    portals.Reserve(pathIndexCount);

    for (uint32_t i = 1; i < pathIndexCount; ++i)
    {
        const uint32_t pastIndex = pathIndices[i - 1];
        const uint32_t curIndex = pathIndices[i];

        const NavigationFace& pastFace = m_faces[pastIndex];
        const NavigationFace& curFace = m_faces[curIndex];

        for (uint32_t j = 0; j < 3; ++j)
        {
            if (pastFace.Connections[j] == curIndex)
            {
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
                if (glm::dot(vertDiff, right) > 0)
                {
                    const Portal port =
                    {
                        .LeftIndex = indexB,
                        .RightIndex = indexA
                    };

                    portals.Push(port);

                    break;
                }

                const Portal port =
                {
                    .LeftIndex = indexA,
                    .RightIndex = indexB
                };

                portals.Push(port);

                break;
            }
        }
    }

    // Pull path tight
    const uint32_t portalCount = portals.Size();

    Array<glm::vec3> path;
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

        const glm::vec3 offset = glm::normalize(rightVertex - leftVertex) * a_agentRadius;

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