// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/RenderAssetStore.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ktx.h>
#include <stb_image.h>

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Rendering/RenderAssetStoreBindings.h"
#include "Rendering/RenderEngine.h"

#include "EngineModelInteropStructures.h"

RenderAssetStore::RenderAssetStore(RenderEngine* a_renderEngine)
{
    m_renderEngine = a_renderEngine;

    m_bindings = new RenderAssetStoreBindings(this);
}
RenderAssetStore::~RenderAssetStore()
{
    delete m_bindings;
}

void RenderAssetStore::Update()
{
    const e_RenderDeviceType device = m_renderEngine->GetDeviceType();

    if (device == RenderDeviceType_DiscreteGPU)
    {
        const uint64_t totalMemory = m_renderEngine->GetTotalDeviceMemory();
        const uint64_t usedMemory = m_renderEngine->GetUsedDeviceMemory();

        // Over half of the VRAM is left so we are wasting out time
        // Better to leave it then trying to reclaim it
        if (totalMemory >> 1 > usedMemory)
        {
            return;
        }
    }

    {
        const Array<bool> state = m_models.ToStateArray();
        TLockArray<RenderAsset> a = m_models.ToLockArray();
        const uint32_t size = state.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == -1)
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
                m_renderEngine->DestroyModel(asset.InternalAddress);
                asset.InternalAddress = -1;
            }
        }
    }
    
    {
        const Array<bool> state = m_textures.ToStateArray();
        TLockArray<RenderAsset> a = m_textures.ToLockArray();
        const uint32_t size = state.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == -1)
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
                m_renderEngine->DestroyTexture(asset.InternalAddress);
                asset.InternalAddress = -1;
            }
        }
    }
}
void RenderAssetStore::Flush()
{
    {   
        const Array<bool> state = m_models.ToStateArray();
        TLockArray<RenderAsset> a = m_models.ToLockArray();
        const uint32_t size = state.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == -1)
            {
                continue;
            }

            m_renderEngine->DestroyModel(asset.InternalAddress);
            asset.InternalAddress = -1;;
        }
    }
    
    {
        const Array<bool> state = m_textures.ToStateArray();
        TLockArray<RenderAsset> a = m_textures.ToLockArray();
        const uint32_t size = state.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset& asset = a[i];
            if (asset.InternalAddress == -1)
            {
                continue;
            }

            m_renderEngine->DestroyTexture(asset.InternalAddress);
            asset.InternalAddress = -1;
        }
    }
}

static void LoadMesh(const aiMesh* a_mesh, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices, float* a_rSqr)
{
    const uint32_t startIndex = a_vertices->Size();

    const bool hasNormals = a_mesh->HasNormals();
    const bool hasTexCoordA = a_mesh->HasTextureCoords(0);
    const bool hasTexCoordB = a_mesh->HasTextureCoords(1);
    const bool hasColour = a_mesh->HasVertexColors(0);

    for (uint32_t i = 0; i < a_mesh->mNumVertices; ++i) 
    {
        Vertex v;

        const aiVector3D& pos = a_mesh->mVertices[i];
        v.Position = glm::vec4(pos.x, -pos.y, pos.z, 1.0f);

        *a_rSqr = glm::max(pos.SquareLength(), *a_rSqr);

        if (hasNormals) 
        {
            const aiVector3D& norm = a_mesh->mNormals[i];
            v.Normal = glm::vec3(norm.x, -norm.y, norm.z);
        }

        if (hasTexCoordA) 
        {
            const aiVector3D& uv = a_mesh->mTextureCoords[0][i];
            v.TexCoordsA = glm::vec2(uv.x, uv.y);
        }

        if (hasTexCoordB)
        {
            const aiVector3D& uv = a_mesh->mTextureCoords[1][i];
            v.TexCoordsB = glm::vec2(uv.x, uv.y);
        }

        if (hasColour) 
        {
            const aiColor4D& colour = a_mesh->mColors[0][i];
            v.Color = glm::vec4(colour.r, colour.g, colour.b, colour.a);
        }

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
            const aiVector3D& posA = a_mesh->mVertices[indexA];
            const aiVector3D& posB = a_mesh->mVertices[indexB];
            const aiVector3D& posC = a_mesh->mVertices[indexC];

            const glm::vec3 pA = glm::vec3(posA.x, -posA.y, posA.z);
            const glm::vec3 pB = glm::vec3(posB.x, -posB.y, posB.z);
            const glm::vec3 pC = glm::vec3(posC.x, -posC.y, posC.z);

            const glm::vec3 diffA = pB - pA;
            const glm::vec3 diffB = pC - pA;

            const glm::vec3 norm = glm::cross(diffA, diffB);

            a_vertices->Ref(startIndex + indexA).Normal += norm;
            a_vertices->Ref(startIndex + indexB).Normal += norm;
            a_vertices->Ref(startIndex + indexC).Normal += norm;
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
            glm::vec3& norm = a_vertices->Ref(i).Normal;

            norm = glm::normalize(norm);
        }
    }
}

static uint32_t LoadBaseModelFile(RenderEngine* a_renderEngine, uint8_t a_data, const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    constexpr uint16_t VertexStride = sizeof(Vertex);

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
            IERROR("Failed reading model data: " + a_path.string());

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, aiProcess_Triangulate | aiProcess_PreTransformVertices, extStr.c_str() + 1);
        IVERIFY(scene != nullptr);
        
        Array<Vertex> vertices;
        Array<uint32_t> indices;
        float radSqr = 0.0f;

        if (a_data != std::numeric_limits<uint8_t>::max())
        {
            IVERIFY(a_data < scene->mNumMeshes);

            LoadMesh(scene->mMeshes[a_data], &vertices, &indices, &radSqr);
        }
        else
        {
            for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
            {
                LoadMesh(scene->mMeshes[i], &vertices, &indices, &radSqr);
            }
        }

        if (vertices.Empty() || indices.Empty() || radSqr <= 0)
        {
            break;
        }

        return a_renderEngine->GenerateModel(vertices.Data(), vertices.Size(), VertexStride, indices.Data(), indices.Size(), glm::sqrt(radSqr));
    }
    default:
    {
        IERROR("Invalid model file extension: " + a_path.string());

        break;
    }
    }

    return -1;
}

uint32_t RenderAssetStore::LoadModel(const std::filesystem::path& a_path, uint32_t a_index)
{
    const RenderAsset asset =
    {
        .Path = a_path.string(),
        .InternalAddress = LoadBaseModelFile(m_renderEngine, (uint8_t)a_index, a_path),
        .Data = (uint8_t)a_index,
    };

    return m_models.PushVal(asset);
}

static void LoadSkinnedMesh(const aiMesh* a_mesh, Array<SkinnedVertex>* a_vertices, Array<uint32_t>* a_indices, const std::unordered_map<std::string, int>& a_boneMap, float* a_rSqr)
{
    for (uint32_t i = 0; i < a_mesh->mNumVertices; ++i) 
    {
        SkinnedVertex v;

        const aiVector3D& pos = a_mesh->mVertices[i];
        v.Position = glm::vec4(pos.x, -pos.y, pos.z, 1.0f);

        *a_rSqr = glm::max(pos.SquareLength(), *a_rSqr);

        if (a_mesh->HasNormals()) 
        {
            const aiVector3D& norm = a_mesh->mNormals[i];
            v.Normal = glm::vec3(norm.x, -norm.y, norm.z);
        }

        if (a_mesh->HasTextureCoords(0)) 
        {
            const aiVector3D& uv = a_mesh->mTextureCoords[0][i];
            v.TexCoords = glm::vec2(uv.x, uv.y);
        }

        if (a_mesh->HasVertexColors(0)) 
        {
            const aiColor4D& colour = a_mesh->mColors[0][i];
            v.Color = glm::vec4(colour.r, colour.g, colour.b, colour.a);
        }

        if (a_mesh->HasBones())
        {
            const aiBone* bone = a_mesh->mBones[i];

            const uint32_t weights = glm::min(uint32_t(4), (uint32_t)bone->mNumWeights);
            for (uint32_t j = 0; j < weights; ++j)
            {
                const auto iter = a_boneMap.find(bone->mName.C_Str());
                if (iter == a_boneMap.end())
                {
                    continue;
                }

                v.BoneIndices[j] = iter->second;
                v.BoneWeights[j] = bone->mWeights[j].mWeight;
            }
        }

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

static uint32_t LoadSkinnedModelFile(RenderEngine* a_renderEngine, uint8_t a_data, const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    constexpr uint16_t VertexStride = sizeof(SkinnedVertex);

    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
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
            IERROR("Failed reading skinned model data: " + a_path.string());

            break;
        }

        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFileFromMemory(dat, (size_t)size, aiProcess_Triangulate | aiProcess_PreTransformVertices, extStr.c_str() + 1);
        IVERIFY(scene != nullptr);
        IVERIFY(scene->mNumSkeletons > 0);

        std::unordered_map<std::string, int> boneMap;

        const aiSkeleton* skeleton = scene->mSkeletons[0];
        for (int i = 0; i < skeleton->mNumBones; ++i)
        {
            const aiSkeletonBone* bone = skeleton->mBones[i];
            const std::string name = bone->mNode->mName.C_Str();

            boneMap.emplace(name, i);
        }

        const aiNode* root = scene->mRootNode;

        Array<SkinnedVertex> vertices;
        Array<uint32_t> indices;
        float radSqr = 0.0f;
        if (a_data != std::numeric_limits<uint8_t>::max())
        {
            IVERIFY(a_data < scene->mNumMeshes);

            LoadSkinnedMesh(scene->mMeshes[a_data], &vertices, &indices, boneMap, &radSqr);
        }
        else
        {
            for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
            {
                LoadSkinnedMesh(scene->mMeshes[i], &vertices, &indices, boneMap, &radSqr);
            }
        }

        if (vertices.Empty() || indices.Empty() || radSqr <= 0)
        {
            IWARN("Empty Model: " + a_path.string());

            break;
        }

        return a_renderEngine->GenerateModel(vertices.Data(), vertices.Size(), VertexStride, indices.Data(), indices.Size(), glm::sqrt(radSqr));
    }
    default:
    {
        IERROR("Invalid skinned model file extension: " + a_path.string());

        break;
    }
    }

    return -1;
}
uint32_t RenderAssetStore::LoadSkinnedModel(const std::filesystem::path& a_path, uint32_t a_index)
{
    const RenderAsset asset =
    {
        .Path = a_path.string(),
        .InternalAddress = LoadSkinnedModelFile(m_renderEngine, (uint8_t)a_index, a_path),
        .Data = (uint8_t)a_index,
        .Flags = 0b1 << RenderAsset::SkinnedBit
    };

    return m_models.PushVal(asset);
}

void RenderAssetStore::DestroyModel(uint32_t a_addr)
{
    IVERIFY(a_addr < m_models.Size());
    IVERIFY(m_models.Exists(a_addr));

    const RenderAsset asset = m_models[a_addr];
    IDEFER(
    if (asset.InternalAddress != -1)
    {
        m_renderEngine->DestroyModel(asset.InternalAddress);
    });
    
    m_models.Erase(a_addr);
}

uint32_t RenderAssetStore::GetModel(uint32_t a_addr)
{
    IVERIFY(a_addr < m_models.Size());
    IVERIFY(m_models.Exists(a_addr));

    TLockArray<RenderAsset> a = m_models.ToLockArray();

    RenderAsset& asset = a[a_addr];
    if (asset.InternalAddress == -1)
    {
        const std::filesystem::path path = asset.Path;

        if (IISBITSET(asset.Flags, RenderAsset::SkinnedBit))
        {
            asset.InternalAddress = LoadSkinnedModelFile(m_renderEngine, asset.Data, path);
        }
        else
        {
            asset.InternalAddress = LoadBaseModelFile(m_renderEngine, asset.Data, path);
        }
    }

    asset.DeReq = 0;
    ISETBIT(asset.Flags, RenderAsset::MarkBit);

    return asset.InternalAddress;
}

uint32_t RenderAssetStore::LoadTexture(const std::filesystem::path& a_path)
{
    FileCache::PreLoad(a_path);

    const RenderAsset asset =
    {
        .Path = a_path.string(),
        .InternalAddress = uint32_t(-1),
    };

    return m_textures.PushVal(asset);
}
void RenderAssetStore::DestroyTexture(uint32_t a_addr)
{
    IVERIFY(a_addr < m_textures.Size());
    IVERIFY(m_textures.Exists(a_addr));

    const RenderAsset asset = m_textures[a_addr];
    IDEFER(
    if (asset.InternalAddress != -1)
    {
        m_renderEngine->DestroyTexture(asset.InternalAddress);
    });

    m_textures.Erase(a_addr);
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
    IVERIFY(a_addr < m_textures.Size());
    IVERIFY(m_textures.Exists(a_addr));

    TLockArray<RenderAsset> a = m_textures.ToLockArray();

    RenderAsset& asset = a[a_addr];
    if (asset.InternalAddress == -1)
    {
        const std::filesystem::path path = asset.Path;
        const std::filesystem::path ext = path.extension();
        const std::string extStr = ext.string();

        switch (StringHash<uint32_t>(extStr.c_str())) 
        {
        case StringHash<uint32_t>(".png"):
        {
            FileHandle* handle = FileCache::LoadFile(path);
            if (handle == nullptr)
            {
                IERROR("GetTexture failed to load file: " + path.string());

                break;
            }

            IDEFER(delete handle);

            const stbi_io_callbacks callbacks = 
            {
                .read = &STBI_FileHandle_Read,
                .skip = &STBI_FileHandle_Skip,
                .eof = &STBI_FileHandle_EOF
            };

            int width;
	    	int height;
	    	int channels;
            stbi_uc* pixels = stbi_load_from_callbacks(&callbacks, handle, &width, &height, &channels, STBI_rgb_alpha);
            if (pixels != nullptr)
            {
                IDEFER(stbi_image_free(pixels));

                asset.InternalAddress = m_renderEngine->GenerateTexture((uint32_t)width, (uint32_t)height, TextureFormat_RGBA, pixels);
            }
            else
            {
                IERROR("GetTexture failed to parse file: " + path.string());
            }

            break;
        }
        case StringHash<uint32_t>(".ktx2"):
        {
            FileHandle* handle = FileCache::LoadFile(path);
            if (handle == nullptr)
            {
                IERROR("GetTexture failed to load file: " + path.string());

                break;
            }

            IDEFER(delete handle);

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
                uint64_t* offsets = new uint64_t[levels];
                IDEFER(delete[] offsets);

                for (uint32_t i = 0; i < levels; ++i)
                {
                    ktx_size_t off;
                    if (ktxTexture_GetImageOffset((ktxTexture*)texture, i, 0, 0, &off) != KTX_SUCCESS)
                    {
                        IERROR("Failed getting KTX offset");
                    }

                    offsets[i] = (uint64_t)off;
                }

                asset.InternalAddress = m_renderEngine->GenerateTextureMipMapped((uint32_t)texture->baseWidth, (uint32_t)texture->baseHeight, levels, offsets, TextureFormat_BC3, texture->pData, (uint64_t)texture->dataSize);
            }
            else
            {
                IERROR("GetTexture failed to parse file: " + path.string());
            }

            break;
        }
        default:
        {
            IERROR("GetTexture invalid file extension: " + path.string());

            break;
        }
        }
    }

    asset.DeReq = 0;
    ISETBIT(asset.Flags, RenderAsset::MarkBit);

    return asset.InternalAddress;
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