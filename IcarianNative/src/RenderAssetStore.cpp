// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/RenderAssetStore.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <meshoptimizer.h>
#include <ktx.h>
#include <stb_image.h>

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/StringUtils.h"
#include "DataTypes/Allocators/BlockAllocator.h"
#include "DataTypes/Allocators/LeakAllocator.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "DataTypes/Allocators/StackAllocator.h"
#include "DataTypes/Allocators/UberAllocator.h"
#include "DataTypes/TStatic.h"
#include "FileCache.h"
#include "IO.h"
#include "IcarianError.h"
#include "Rendering/RenderAssetStoreBindings.h"
#include "Rendering/RenderEngine.h"
#include "Runtime/RuntimeManager.h"

static TStatic<uint32_t> ScratchAllocator = TStatic<uint32_t>();

RenderAssetStore::RenderAssetStore(RenderEngine* a_renderEngine)
{
    m_blockAllocator = MallocAllocator::Instance->Create<BlockAllocator>(BlockAllocatorSize, UberAllocator::Instance);
#ifdef DEBUG
    m_blockAllocator = MallocAllocator::Instance->Create<LeakAllocator>(m_blockAllocator);
#endif

    m_data = m_blockAllocator->ZTAllocate<ClassData>();
    m_data->StackAllocators = Array<RenderAssetScratchAllocator>(m_blockAllocator);

    m_data->Renderer = a_renderEngine;

    m_data->Bindings = m_blockAllocator->Create<RenderAssetStoreBindings>(this);
}
RenderAssetStore::~RenderAssetStore()
{
    m_blockAllocator->Destroy(m_data->Bindings);

    for (const RenderAssetScratchAllocator& a : m_data->StackAllocators)
    {
        if (a.Allocator != nullptr)
        {
            m_blockAllocator->Destroy(a.Allocator);
        }
    }

    m_blockAllocator->Destroy(m_data);

#ifdef DEBUG
    Allocator* upstreamAllocator = ((LeakAllocator*)m_blockAllocator)->GetUpstreamAllocator();
    IDEFER(MallocAllocator::Instance->Destroy(upstreamAllocator));
#endif
    MallocAllocator::Instance->Destroy(m_blockAllocator);
}

void RenderAssetStore::Update()
{
    const e_RenderDeviceType device = m_data->Renderer->GetDeviceType();

    if (device == RenderDeviceType_DiscreteGPU)
    {
        const uint64_t totalMemory = m_data->Renderer->GetTotalDeviceMemory();
        const uint64_t usedMemory = m_data->Renderer->GetUsedDeviceMemory();

        // Over half of the VRAM is left so we are wasting out time
        // Better to leave it then trying to reclaim it
        if (totalMemory >> 1 > usedMemory)
        {
            return;
        }
    }

    // Need to get the index of the scratch allocator for this thread
    const uint32_t scratchIndex = GetScratchAllocatorIndex();

    StackAllocator* scratchAllocator = ILAMBDA(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        ++alloc.Count;

        ILRETURN alloc.Allocator;
    });
    IDEFER(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        --alloc.Count;
    });

    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        // No need to waste memory just use a packed state
        const uint32_t size = m_data->Meshes.Size();
        const Array<uint8_t> state = m_data->Meshes.ToPackedStateArray(scratchAllocator);
        TLockArray<RenderAsset> a = m_data->Meshes.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == uint32_t(-1))
            {
                continue;
            }

            if (IISBITSET(asset.Flags, RenderAsset::MarkBit))
            {
                asset.DeReq = 0;
                ICLEARBIT(asset.Flags, RenderAsset::MarkBit);
            }
            else
            {
                ++asset.DeReq;
            }

            if (asset.DeReq > RenderAssetStore::DeReqCount)
            {
                m_data->Renderer->DestroyMesh(asset.InternalAddress);
                asset.InternalAddress = -1;
            }
        }
    }

    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        const uint32_t size = m_data->Models.Size();
        const Array<uint8_t> state = m_data->Models.ToPackedStateArray(scratchAllocator);
        TLockArray<RenderAsset> a = m_data->Models.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == uint32_t(-1))
            {
                continue;
            }

            if (IISBITSET(asset.Flags, RenderAsset::MarkBit))
            {
                asset.DeReq = 0;
                ICLEARBIT(asset.Flags, RenderAsset::MarkBit);
            }
            else
            {
                ++asset.DeReq;
            }

            if (asset.DeReq > RenderAssetStore::DeReqCount)
            {
                m_data->Renderer->DestroyModel(asset.InternalAddress);
                asset.InternalAddress = -1;
            }
        }
    }

    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        const uint32_t size = m_data->Textures.Size();
        const Array<uint8_t> state = m_data->Textures.ToPackedStateArray(scratchAllocator);
        TLockArray<RenderAsset> a = m_data->Textures.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == uint32_t(-1))
            {
                continue;
            }

            if (IISBITSET(asset.Flags, RenderAsset::MarkBit))
            {
                asset.DeReq = 0;
                ICLEARBIT(asset.Flags, RenderAsset::MarkBit);
            }
            else
            {
                ++asset.DeReq;
            }

            if (asset.DeReq > RenderAssetStore::DeReqCount)
            {
                m_data->Renderer->DestroyTexture(asset.InternalAddress);
                asset.InternalAddress = -1;
            }
        }
    }
}
void RenderAssetStore::Flush()
{
    const uint32_t scratchIndex = GetScratchAllocatorIndex();

    StackAllocator* scratchAllocator = ILAMBDA(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        ++alloc.Count;

        ILRETURN alloc.Allocator;
    });
    IDEFER(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        --alloc.Count;
    });

    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        const uint32_t size = m_data->Meshes.Size();
        const Array<uint8_t> state = m_data->Meshes.ToPackedStateArray(scratchAllocator);
        TLockArray<RenderAsset> a = m_data->Meshes.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == uint32_t(-1))
            {
                continue;
            }

            m_data->Renderer->DestroyMesh(asset.InternalAddress);
            asset.InternalAddress = -1;
        }
    }

    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        const uint32_t size = m_data->Models.Size();
        const Array<uint8_t> state = m_data->Models.ToPackedStateArray(scratchAllocator);
        TLockArray<RenderAsset> a = m_data->Models.ToLockArray();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == uint32_t(-1))
            {
                continue;
            }

            m_data->Renderer->DestroyModel(asset.InternalAddress);
            asset.InternalAddress = -1;
        }
    }

    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        const Array<bool> state = m_data->Textures.ToStateArray(scratchAllocator);
        TLockArray<RenderAsset> a = m_data->Textures.ToLockArray();
        const uint32_t size = state.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            const uint32_t index = i / 8;
            const uint32_t offset = i % 8;

            if (!IISBITSET(state[index], offset))
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == uint32_t(-1))
            {
                continue;
            }

            m_data->Renderer->DestroyTexture(asset.InternalAddress);
            asset.InternalAddress = -1;
        }
    }
}

static void AILoadMesh(const aiMesh* a_mesh, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices, float* a_rSqr)
{
    const uint32_t startIndex = a_vertices->Size();

    const bool hasNormals = a_mesh->HasNormals();
    const bool hasTexCoordA = a_mesh->HasTextureCoords(0);
    const bool hasTexCoordB = a_mesh->HasTextureCoords(1);
    const bool hasColour = a_mesh->HasVertexColors(0);

    for (uint32_t i = 0; i < a_mesh->mNumVertices; ++i) 
    {
        const Vertex v = 
        {
            .Position = ILAMBDA(
            {
                const aiVector3D& p = a_mesh->mVertices[i];

                ILRETURN glm::vec4(p.x, -p.y, p.z, 1.0f);
            }),
            .Normal = ILAMBDA(
            {
                if (hasNormals)
                {
                    const aiVector3D& n = a_mesh->mNormals[i];

                    ILRETURN glm::vec4(n.x, -n.y, n.z, 0.0f);
                }

                ILRETURN glm::vec4(0.0f);
            }),
            .Color = ILAMBDA(
            {
                if (hasColour)
                {
                    const aiColor4D& colour = a_mesh->mColors[0][i];

                    ILRETURN glm::vec4(colour.r, colour.g, colour.b, colour.a);
                }

                ILRETURN glm::vec4(1.0f);
            }),
            .TexCoordsA = ILAMBDA(
            {
                if (hasTexCoordA)
                {
                    const aiVector3D& u = a_mesh->mTextureCoords[0][i];

                    ILRETURN glm::vec2(u.x, u.y);
                }

                ILRETURN glm::vec2(0.0f);
            }),
            .TexCoordsB = ILAMBDA(
            {
                if (hasTexCoordB)
                {
                    const aiVector3D& u = a_mesh->mTextureCoords[1][i];

                    ILRETURN glm::vec2(u.x, u.y);
                }

                ILRETURN glm::vec2(0.0f);
            })
        };

        *a_rSqr = glm::max(glm::dot(v.Position.xyz(), v.Position.xyz()), *a_rSqr);

        a_vertices->Push(v);
    }

    for (uint32_t i = 0; i < a_mesh->mNumFaces; ++i) 
    {
        const aiFace& face = a_mesh->mFaces[i];

        const uint32_t indexA = face.mIndices[0];
        const uint32_t indexB = face.mIndices[2];
        const uint32_t indexC = face.mIndices[1];

        if (!hasNormals)
        {
            Vertex& vA = (*a_vertices)[startIndex + indexA];
            Vertex& vB = (*a_vertices)[startIndex + indexB];
            Vertex& vC = (*a_vertices)[startIndex + indexC];

            const glm::vec3 pA = vA.Position.xyz();
            const glm::vec3 pB = vB.Position.xyz();
            const glm::vec3 pC = vC.Position.xyz();

            const glm::vec3 diffA = pB - pA;
            const glm::vec3 diffB = pC - pA;

            const glm::vec3 norm = glm::cross(diffA, diffB);

            vA.Normal += glm::vec4(norm, 0.0f);
            vB.Normal += glm::vec4(norm, 0.0f);
            vC.Normal += glm::vec4(norm, 0.0f);
        }

        a_indices->Push(indexA);
        a_indices->Push(indexB);
        a_indices->Push(indexC);
    }

    if (!hasNormals)
    {
        const uint32_t count = a_vertices->Size();
        for (uint32_t i = startIndex; i < count; ++i)
        {
            glm::vec4& norm = (*a_vertices)[i].Normal;

            norm = glm::vec4(glm::normalize(norm.xyz()), 0.0f);
        }
    }
}

bool RenderAssetStore::LoadModelData(const COWU8String& a_path, uint8_t a_data, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices, float* a_radius)
{
    IERRBLOCK;

    const uint32_t scratchIndex = GetScratchAllocatorIndex();

    StackAllocator* scratchAllocator = ILAMBDA(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        ++alloc.Count;

        ILRETURN alloc.Allocator;
    });
    IDEFER(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        --alloc.Count;
    });

    const COWU8String extStr = IO::GetExtension(a_path, scratchAllocator);

    // TODO: Create and handle pre optimized files
    switch (StringHash<uint32_t>(extStr.CStr()))
    {
    case StringHash<uint32_t>(".obj"):
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        FileHandle* handle = FileCache::LoadFile(a_path);
        IERRCHECKRET(handle != nullptr, false);
        IDEFER(MallocAllocator::Instance->Destroy(handle));

        const uint64_t size = handle->GetSize();
        uint8_t* dat = scratchAllocator->TAllocate<uint8_t>(size);
        IERRCHECKRET(handle->Read(dat, size) == size, false);

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory
        (
            dat,
            (size_t)size,
            aiProcess_Triangulate | aiProcess_PreTransformVertices,
            extStr.CStr() + 1
        );
        IERRCHECKRET(scene != nullptr, false);

        float radSqr = 0.0f;

        if (a_data != std::numeric_limits<uint8_t>::max())
        {
            IERRCHECKRET(a_data < scene->mNumMeshes, false);

            AILoadMesh(scene->mMeshes[a_data], a_vertices, a_indices, &radSqr);
        }
        else
        {
            for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
            {
                AILoadMesh(scene->mMeshes[i], a_vertices, a_indices, &radSqr);
            }
        }

        constexpr uint32_t VertexSize = sizeof(Vertex);

        const uint32_t indexCount = a_indices->Size();
        const uint32_t vertexCount = a_vertices->Size();

        meshopt_optimizeVertexCache
        (
            a_indices->Data(),
            a_indices->Data(),
            indexCount,
            vertexCount
        );
        meshopt_optimizeOverdraw
        (
            a_indices->Data(),
            a_indices->Data(),
            indexCount,
            &((*a_vertices)[0].Position.x),
            vertexCount,
            VertexSize,
            1.05f
        );
        const size_t newVertexCount = meshopt_optimizeVertexFetch
        (
            a_vertices->Data(),
            a_indices->Data(),
            indexCount,
            a_vertices->Data(),
            vertexCount,
            VertexSize
        );

        a_vertices->Resize((uint32_t)newVertexCount);

        *a_radius = glm::sqrt(radSqr);

        return true;
    }
    default:
    {
        IERROR("Invalid model file extension: " + a_path);

        break;
    }
    }

    return false;
}

uint32_t RenderAssetStore::LoadMeshData(const COWU8String& a_path, uint8_t a_index)
{
    IERRBLOCK;

    const uint32_t scratchIndex = GetScratchAllocatorIndex();

    StackAllocator* scratchAllocator = ILAMBDA(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        ++alloc.Count;

        ILRETURN alloc.Allocator;
    });
    IDEFER(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        --alloc.Count;
    });

    scratchAllocator->PushStackPointer();
    IDEFER(scratchAllocator->PopStackPointer());

    Array<Vertex> vertices = Array<Vertex>(m_blockAllocator);
    Array<uint32_t> indices = Array<uint32_t>(m_blockAllocator);
    float radius;
    IERRCHECKRET(LoadModelData(a_path, a_index, &vertices, &indices, &radius), -1);

    IERRCHECKRET(radius > 0, -1);

    const uint32_t vertexCount = vertices.Size();
    IERRCHECKRET(vertexCount > 0, -1);

    const uint32_t indexCount = indices.Size();
    IERRCHECKRET(indexCount > 0, -1);

    constexpr uint32_t MeshletTriangleCount = 124;
    constexpr uint32_t MeshletVertexCount = 64;

    const size_t maxMeshletCount = meshopt_buildMeshletsBound(indexCount, MeshletVertexCount, MeshletTriangleCount);
    IERRCHECKRET(maxMeshletCount > 0, -1);

    meshopt_Meshlet* meshoptMeshlets = scratchAllocator->TAllocate<meshopt_Meshlet>(maxMeshletCount);

    uint32_t* meshletVertices = scratchAllocator->TAllocate<uint32_t>(maxMeshletCount * MeshletVertexCount);
    IDEFER(scratchAllocator->Free(meshletVertices));
    uint8_t* meshletTriangles = scratchAllocator->TAllocate<uint8_t>(maxMeshletCount * MeshletTriangleCount * 3);
    IDEFER(scratchAllocator->Free(meshletTriangles));

    const size_t meshletCount = meshopt_buildMeshlets
    (
        meshoptMeshlets,
        meshletVertices,
        meshletTriangles,
        indices.Data(),
        indexCount,
        (float*)vertices.Data(),
        vertexCount,
        sizeof(Vertex),
        MeshletVertexCount,
        MeshletTriangleCount,
        0.0f
    );

    IcarianCore::ShaderMeshletBuffer* meshlets = scratchAllocator->TAllocate<IcarianCore::ShaderMeshletBuffer>(meshletCount);

    uint32_t meshletVertexCount = 0;
    uint32_t meshletTriangleCount = 0;
    for (size_t i = 0; i < meshletCount; ++i)
    {
        const meshopt_Meshlet& m = meshoptMeshlets[i];

        uint8_t* mTriangles = meshletTriangles + m.triangle_offset;
        uint32_t* mVertices = meshletVertices + m.vertex_offset;

        meshletVertexCount += m.vertex_count;
        meshletTriangleCount += m.triangle_count;

        meshopt_optimizeMeshlet(mVertices, mTriangles, m.triangle_count, m.vertex_count);

        glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());

        for (uint32_t j = 0; j < m.vertex_count; ++j)
        {
            const uint32_t index = (uint32_t)meshletVertices[i];

            const float* vPtr = (float*)(vertices.Data() + index);

            max.x = glm::max(vPtr[0], max.x);
            max.y = glm::max(vPtr[1], max.y);
            max.z = glm::max(vPtr[2], max.z);

            min.x = glm::min(vPtr[0], min.x);
            min.y = glm::min(vPtr[1], min.y);
            min.z = glm::min(vPtr[2], min.z);
        }

        const glm::vec3 bounds = max - min;
        const glm::vec3 halfBounds = bounds * 0.5f;

        const glm::vec3 center = min + halfBounds;
        const float radius = glm::length(halfBounds);

        meshlets[i].Data = glm::uvec4(m.vertex_offset, m.triangle_offset, m.vertex_count, m.triangle_count);
        meshlets[i].Bounds = glm::vec4(center, radius);
    }

    return m_data->Renderer->GenerateMesh
    (
        vertices.Data(),
        vertexCount,
        sizeof(Vertex),
        meshletVertices,
        meshletVertexCount,
        meshletTriangles,
        meshletTriangleCount,
        meshlets,
        meshletCount,
        radius
    );
}
uint32_t RenderAssetStore::LoadMesh(const COWU8String& a_path, uint8_t a_index)
{
    const uint32_t addr = LoadMeshData(a_path, a_index);
    if (addr == uint32_t(-1))
    {
        return -1;
    }

    const RenderAsset asset =
    {
        .Path = COWU8String(a_path, m_blockAllocator),
        .InternalAddress = addr,
        // .InternalAddress = uint32_t(-1),
        .Data = a_index,
    };

    return m_data->Meshes.PushVal(asset);
}
void RenderAssetStore::DestroyMesh(uint32_t a_addr)
{
    IVERIFY(m_data->Meshes.Exists(a_addr));

    const RenderAsset asset = m_data->Meshes[a_addr];
    IDEFER(
    if (asset.InternalAddress != uint32_t(-1))
    {
        m_data->Renderer->DestroyMesh(asset.InternalAddress);
    });

    m_data->Meshes.Erase(a_addr);
}
uint32_t RenderAssetStore::GetMesh(uint32_t a_addr)
{
    IVERIFY(m_data->Meshes.Exists(a_addr));

    TLockArray<RenderAsset> a = m_data->Meshes.ToLockArray();

    RenderAsset& asset = a[a_addr];
    if (asset.InternalAddress == uint32_t(-1))
    {
        asset.InternalAddress = LoadMeshData(asset.Path, asset.Data);
    }

    asset.DeReq = 0;
    ISETBIT(asset.Flags, RenderAsset::MarkBit);

    return asset.InternalAddress;
}

uint32_t RenderAssetStore::LoadModel(const COWU8String& a_path, uint8_t a_index)
{
    constexpr uint16_t VertexStride = sizeof(Vertex);

    Array<Vertex> vertices = Array<Vertex>(m_blockAllocator);
    Array<uint32_t> indices = Array<uint32_t>(m_blockAllocator);
    float radius;
    if (!LoadModelData(a_path, a_index, &vertices, &indices, &radius))
    {
        return -1;
    }

    if (vertices.Empty() || indices.Empty() || radius <= 0)
    {
        return -1;
    }

    const uint32_t modelAddr = m_data->Renderer->GenerateModel
    (
        vertices.Data(),
        vertices.Size(),
        VertexStride,
        indices.Data(),
        indices.Size(),
        radius
    );
    if (modelAddr == uint32_t(-1))
    {
        return -1;
    }

    const RenderAsset asset =
    {
        .Path = COWU8String(a_path, m_blockAllocator),
        .InternalAddress = modelAddr,
        .Data = (uint8_t)a_index,
    };

    return m_data->Models.PushVal(asset);
}

static void LoadSkinnedMesh
(
    const aiMesh* a_mesh,
    Array<SkinnedVertex>* a_vertices,
    Array<uint32_t>* a_indices,
    const Dictionary<COWU8String, int>& a_boneMap,
    float* a_rSqr,
    Allocator* a_tempAllocator
)
{
    const bool hasNormal = a_mesh->HasNormals();
    const bool hasUV = a_mesh->HasTextureCoords(0);
    const bool hasVertexColour = a_mesh->HasVertexColors(0);
    const bool hasBones = a_mesh->HasBones();

    for (uint32_t i = 0; i < a_mesh->mNumVertices; ++i) 
    {
        const SkinnedVertex v =
        {
            .Position = ILAMBDA(
            {
                const aiVector3D& p = a_mesh->mVertices[i];

                ILRETURN glm::vec4(p.x, -p.y, p.z, 1.0f);
            }),
            .Normal = ILAMBDA(
            {
                if (hasNormal)
                {
                    const aiVector3D& n = a_mesh->mNormals[i];

                    ILRETURN glm::vec4(n.x, -n.y, n.z, 0.0f);
                }

                ILRETURN glm::vec4(0.0f);
            }),
            .Color = ILAMBDA(
            {
                if (hasVertexColour)
                {
                    const aiColor4D& colour = a_mesh->mColors[0][i];

                    ILRETURN glm::vec4(colour.r, colour.g, colour.b, colour.a);
                }

                ILRETURN glm::vec4(1.0f);
            }),
            .TexCoords = ILAMBDA(
            {
                if (hasUV)
                {
                    const aiVector3D& u = a_mesh->mTextureCoords[0][i];

                    ILRETURN glm::vec2(u.x, u.y);
                }

                ILRETURN glm::vec2(0.0f);
            }),
            .BoneWeights = ILAMBDA(
            {
                if (hasBones)
                {
                    const aiBone* bone = a_mesh->mBones[i];

                    glm::vec4 w = glm::vec4(0.0f);

                    const uint32_t weights = glm::min(uint32_t(4), (uint32_t)bone->mNumWeights);
                    for (uint32_t j = 0; j < weights; ++j)
                    {
                        w[j] = bone->mWeights[j].mWeight;
                    }

                    ILRETURN w;
                }

                ILRETURN glm::vec4(0.0f);
            }),
            .BoneIndices = ILAMBDA(
            {
                if (hasBones)
                {
                    const aiBone* bone = a_mesh->mBones[i];

                    glm::ivec4 b = glm::ivec4(0);

                    const uint32_t weights = glm::min(uint32_t(4), (uint32_t)bone->mNumWeights);
                    for (uint32_t j = 0; j < weights; ++j)
                    {
                        const COWU8String str = COWU8String(bone->mName.C_Str(), bone->mName.length, a_tempAllocator);
                        if (!a_boneMap.Exists(str))
                        {
                            continue;
                        }

                        b[j] = a_boneMap[str];
                    }

                    ILRETURN b;
                }

                ILRETURN glm::ivec4(0);
            })
        };

        *a_rSqr = glm::max(glm::length2(v.Position.xyz()), *a_rSqr);

        a_vertices->Push(v);
    }

    for (uint32_t i = 0; i < a_mesh->mNumFaces; ++i) 
    {
        const aiFace& face = a_mesh->mFaces[i];

        a_indices->Push(face.mIndices[0]);
        a_indices->Push(face.mIndices[2]);
        a_indices->Push(face.mIndices[1]);
    }
}

uint32_t RenderAssetStore::LoadSkinnedModelFile(RenderEngine* a_renderEngine, uint8_t a_data, const COWU8String& a_path)
{
    IERRBLOCK;

    const uint32_t scratchIndex = GetScratchAllocatorIndex();

    StackAllocator* scratchAllocator = ILAMBDA(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        ++alloc.Count;

        ILRETURN alloc.Allocator;
    });
    IDEFER(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        --alloc.Count;
    });

    const COWU8String ext = IO::GetExtension(a_path, scratchAllocator);

    constexpr uint16_t VertexStride = sizeof(SkinnedVertex);

    switch (StringHash<uint32_t>(ext.CStr()))
    {
    case StringHash<uint32_t>(".dae"):
    case StringHash<uint32_t>(".fbx"):
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        scratchAllocator->PushStackPointer();
        IDEFER(scratchAllocator->PopStackPointer());

        FileHandle* handle = FileCache::LoadFile(a_path);
        IERRCHECKRET(handle != nullptr, -1);
        IDEFER(MallocAllocator::Instance->Destroy(handle));

        const uint64_t size = handle->GetSize();
        uint8_t* dat = scratchAllocator->TAllocate<uint8_t>(size);

        IERRCHECKRET(handle->Read(dat, size) != size, -1);

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory
        (
            dat,
            (size_t)size,
            aiProcess_Triangulate | aiProcess_PreTransformVertices,
            ext.CStr() + 1
        );
        IERRCHECKRET(scene != nullptr, -1);
        IERRCHECKRET(scene->mNumSkeletons > 0, -1);

        Dictionary<COWU8String, int> boneMap = Dictionary<COWU8String, int>(scratchAllocator);

        const aiSkeleton* skeleton = scene->mSkeletons[0];
        for (unsigned int i = 0; i < skeleton->mNumBones; ++i)
        {
            const aiSkeletonBone* bone = skeleton->mBones[i];
            const aiString& boneName = bone->mNode->mName;
            const COWU8String name = COWU8String(boneName.C_Str(), boneName.length, scratchAllocator);

            boneMap.Push(name, i);
        }

        Array<SkinnedVertex> vertices = Array<SkinnedVertex>(m_blockAllocator);
        Array<uint32_t> indices = Array<uint32_t>(m_blockAllocator);
        float radSqr = 0.0f;
        if (a_data != std::numeric_limits<uint8_t>::max())
        {
            IERRCHECKRET(a_data < scene->mNumMeshes, -1);

            scratchAllocator->PushStackPointer();
            IDEFER(scratchAllocator->PopStackPointer());

            LoadSkinnedMesh(scene->mMeshes[a_data], &vertices, &indices, boneMap, &radSqr, scratchAllocator);
        }
        else
        {
            for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
            {
                scratchAllocator->PushStackPointer();
                IDEFER(scratchAllocator->PopStackPointer());

                LoadSkinnedMesh(scene->mMeshes[i], &vertices, &indices, boneMap, &radSqr, scratchAllocator);
            }
        }

        if (vertices.Empty() || indices.Empty() || radSqr <= 0)
        {
            IWARN("Empty Model: " + a_path);

            break;
        }

        return a_renderEngine->GenerateModel
        (
            vertices.Data(),
            vertices.Size(),
            VertexStride,
            indices.Data(),
            indices.Size(),
            glm::sqrt(radSqr)
        );
    }
    default:
    {
        break;
    }
    }

    return -1;
}
uint32_t RenderAssetStore::LoadSkinnedModel(const COWU8String& a_path, uint8_t a_index)
{
    const uint32_t internalAddr = LoadSkinnedModelFile(m_data->Renderer, (uint8_t)a_index, a_path);
    if (internalAddr == uint32_t(-1))
    {
        return -1;
    }

    const RenderAsset asset =
    {
        .Path = COWU8String(a_path, m_blockAllocator),
        .InternalAddress = internalAddr,
        .Data = (uint8_t)a_index,
        .Flags = 0b1 << RenderAsset::SkinnedBit
    };

    return m_data->Models.PushVal(asset);
}

void RenderAssetStore::DestroyModel(uint32_t a_addr)
{
    IVERIFY(m_data->Models.Exists(a_addr));

    const RenderAsset asset = m_data->Models[a_addr];
    IDEFER(
    if (asset.InternalAddress != uint32_t(-1))
    {
        m_data->Renderer->DestroyModel(asset.InternalAddress);
    });

    m_data->Models.Erase(a_addr);
}

uint32_t RenderAssetStore::GetModel(uint32_t a_addr)
{
    IVERIFY(m_data->Models.Exists(a_addr));

    TLockArray<RenderAsset> a = m_data->Models.ToLockArray();

    RenderAsset& asset = a[a_addr];
    if (asset.InternalAddress == uint32_t(-1))
    {
        if (IISBITSET(asset.Flags, RenderAsset::SkinnedBit))
        {
            asset.InternalAddress = LoadSkinnedModelFile(m_data->Renderer, asset.Data, asset.Path);
        }
        else
        {
            constexpr uint16_t VertexStride = sizeof(Vertex);

            Array<Vertex> vertices = Array<Vertex>(m_blockAllocator);
            Array<uint32_t> indices = Array<uint32_t>(m_blockAllocator);
            float radius;
            if (!LoadModelData(asset.Path, asset.Data, &vertices, &indices, &radius))
            {
                return -1;
            }

            if (vertices.Empty() || indices.Empty() || radius <= 0)
            {
                return -1;
            }

            asset.InternalAddress = m_data->Renderer->GenerateModel
            (
                vertices.Data(),
                vertices.Size(),
                VertexStride,
                indices.Data(),
                indices.Size(),
                radius
            );
        }
    }

    asset.DeReq = 0;
    ISETBIT(asset.Flags, RenderAsset::MarkBit);

    return asset.InternalAddress;
}

uint32_t RenderAssetStore::LoadTexture(const COWU8String& a_path)
{
    const RenderAsset asset =
    {
        .Path = COWU8String(a_path, m_blockAllocator),
        .InternalAddress = uint32_t(-1),
    };

    return m_data->Textures.PushVal(asset);
}
void RenderAssetStore::DestroyTexture(uint32_t a_addr)
{
    IVERIFY(m_data->Textures.Exists(a_addr));

    const RenderAsset asset = m_data->Textures[a_addr];
    IDEFER(
    if (asset.InternalAddress != uint32_t(-1))
    {
        m_data->Renderer->DestroyTexture(asset.InternalAddress);
    });

    m_data->Textures.Erase(a_addr);
}

static int STBI_FileHandle_Read(void* a_user, char* a_data, int a_size)
{
    IVERIFY(a_user != NULL);
    FileHandle* handle = (FileHandle*)a_user;

    return (int)handle->Read(a_data, (uint64_t)a_size);
}
static void STBI_FileHandle_Skip(void* a_user, int a_n)
{
    IVERIFY(a_user != NULL);
    FileHandle* handle = (FileHandle*)a_user;

    handle->Ignore(a_n);
}
static int STBI_FileHandle_EOF(void* a_user)
{
    IVERIFY(a_user != NULL);
    const FileHandle* handle = (FileHandle*)a_user;

    return (int)handle->EndOfFile();
}

static KTX_error_code KTX_FileHandle_Read(ktxStream* a_stream, void* a_dst, const ktx_size_t a_count)
{
    FileHandle* handle = (FileHandle*)a_stream->data.custom_ptr.address;

    handle->Read(a_dst, (uint64_t)a_count);

    return KTX_SUCCESS;
}
static KTX_error_code KTX_FileHandle_Write(ktxStream* a_stream, const void* a_src, const ktx_size_t a_size, const ktx_size_t a_cout)
{
    // Should not occur implementing for safety
    IERROR("KTX is attempting to write");

    return KTX_INVALID_OPERATION;
}
static KTX_error_code KTX_FileHandle_Skip(ktxStream* a_stream, const ktx_size_t a_count)
{
    FileHandle* handle = (FileHandle*)a_stream->data.custom_ptr.address;

    handle->Ignore((uint64_t)a_count);

    return KTX_SUCCESS;
}
static KTX_error_code KTX_FileHandle_GetPos(ktxStream* a_stream, ktx_off_t* const a_offset)
{
    const FileHandle* handle = (FileHandle*)a_stream->data.custom_ptr.address;

    *a_offset = (ktx_off_t)handle->GetOffset();

    return KTX_SUCCESS;
}
static KTX_error_code KTX_FileHandle_SetPos(ktxStream* a_stream, const ktx_off_t a_offset)
{
    FileHandle* handle = (FileHandle*)a_stream->data.custom_ptr.address;

    handle->Seek(a_offset);

    return KTX_SUCCESS;
}
static KTX_error_code KTX_FileHandle_GetSize(ktxStream* a_stream, ktx_size_t* const a_size)
{
    FileHandle* handle = (FileHandle*)a_stream->data.custom_ptr.address;

    *a_size = (ktx_size_t)handle->GetSize();

    return KTX_SUCCESS;
}
static void KTX_FileHandle_Destruct(ktxStream* a_stream)
{
    // Not passing ownership to the stream so nothing to do
}

uint32_t RenderAssetStore::GetTexture(uint32_t a_addr)
{
    IVERIFY(m_data->Textures.Exists(a_addr));

    const uint32_t scratchIndex = GetScratchAllocatorIndex();

    StackAllocator* scratchAllocator = ILAMBDA(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        ++alloc.Count;

        ILRETURN alloc.Allocator;
    });
    IDEFER(
    {
        const ThreadGuard g = ThreadGuard(m_scratchLock);

        RenderAssetScratchAllocator& alloc = m_data->StackAllocators[scratchIndex];

        --alloc.Count;
    });

    TLockArray<RenderAsset> a = m_data->Textures.ToLockArray();

    RenderAsset& asset = a[a_addr];
    if (asset.InternalAddress == uint32_t(-1))
    {
        const COWU8String ext = IO::GetExtension(asset.Path, scratchAllocator);

        switch (StringHash<uint32_t>(ext.CStr()))
        {
        case StringHash<uint32_t>(".png"):
        {
            FileHandle* handle = FileCache::LoadFile(asset.Path);
            if (handle == nullptr)
            {
                IERROR("GetTexture failed to load file: " + asset.Path);

                break;
            }
            IDEFER(MallocAllocator::Instance->Destroy(handle));

            const stbi_io_callbacks callbacks = 
            {
                .read = &STBI_FileHandle_Read,
                .skip = &STBI_FileHandle_Skip,
                .eof = &STBI_FileHandle_EOF
            };

            int width;
	    	int height;
	    	int channels;
            stbi_uc* pixels = stbi_load_from_callbacks
            (
                &callbacks,
                handle,
                &width,
                &height,
                &channels,
                STBI_rgb_alpha
            );
            if (pixels != nullptr)
            {
                IDEFER(stbi_image_free(pixels));

                asset.InternalAddress = m_data->Renderer->GenerateTexture
                (
                    (uint32_t)width,
                    (uint32_t)height,
                    TextureFormat_RGBA,
                    pixels
                );
            }
            else
            {
                IERROR("GetTexture failed to parse file: " + asset.Path);
            }

            break;
        }
        case StringHash<uint32_t>(".ktx2"):
        {
            scratchAllocator->PushStackPointer();
            IDEFER(scratchAllocator->PopStackPointer());

            FileHandle* handle = FileCache::LoadFile(asset.Path);
            if (handle == nullptr)
            {
                IERROR("GetTexture failed to load file: " + asset.Path);

                break;
            }
            IDEFER(MallocAllocator::Instance->Destroy(handle));

            ktxStream stream =
            {
                .read = &KTX_FileHandle_Read,
                .skip = &KTX_FileHandle_Skip,
                .write = &KTX_FileHandle_Write,
                .getpos = &KTX_FileHandle_GetPos,
                .setpos = &KTX_FileHandle_SetPos,
                .getsize = &KTX_FileHandle_GetSize,
                .destruct = &KTX_FileHandle_Destruct,
                .type = eStreamTypeCustom,
                .data =
                {
                    .custom_ptr =
                    {
                        .address = handle
                    }
                },
                .closeOnDestruct = KTX_FALSE
            };

            ktxTexture2* texture;
            if (ktxTexture2_CreateFromStream(&stream, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture) == KTX_SUCCESS)
            {
                IDEFER(ktxTexture_Destroy((ktxTexture*)texture));

                if (ktxTexture2_NeedsTranscoding(texture))
                {
                    if (ktxTexture2_TranscodeBasis(texture, KTX_TTF_BC3_RGBA, 0) != KTX_SUCCESS)
                    {
                        IERROR("Failed to transcode KTX texture");
                    }
                }

                const uint32_t levels = (uint32_t)texture->numLevels;
                uint64_t* offsets = scratchAllocator->TAllocate<uint64_t>(levels);

                for (uint32_t i = 0; i < levels; ++i)
                {
                    ktx_size_t off;
                    if (ktxTexture_GetImageOffset((ktxTexture*)texture, i, 0, 0, &off) != KTX_SUCCESS)
                    {
                        IERROR("Failed getting KTX offset");
                    }

                    offsets[i] = (uint64_t)off;
                }

                asset.InternalAddress = m_data->Renderer->GenerateTextureMipMapped
                (
                    (uint32_t)texture->baseWidth,
                    (uint32_t)texture->baseHeight,
                    levels,
                    offsets,
                    TextureFormat_BC3,
                    texture->pData,
                    (uint64_t)texture->dataSize
                );
            }
            else
            {
                IERROR("GetTexture failed to parse file: " + asset.Path);
            }

            break;
        }
        default:
        {
            IERROR("GetTexture invalid file extension: " + asset.Path);

            break;
        }
        }
    }

    asset.DeReq = 0;
    ISETBIT(asset.Flags, RenderAsset::MarkBit);

    return asset.InternalAddress;
}

uint32_t RenderAssetStore::GetScratchAllocatorIndex()
{
    if (!ScratchAllocator.Exists())
    {
        if (m_data->ScratchIndex >= m_data->StackAllocators.Size())
        {
            StackAllocator* allocator = m_blockAllocator->Create<StackAllocator>(ScratchAllocatorSize, UberAllocator::Instance);

            const RenderAssetScratchAllocator data =
            {
                .Allocator = allocator
            };

            m_data->StackAllocators.Push(data);
        }

        ScratchAllocator.Push(m_data->ScratchIndex++);
    }

    return *ScratchAllocator;
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
