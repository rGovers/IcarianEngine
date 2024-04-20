#include "Rendering/RenderAssetStore.h"

#include <ktx.h>
#include <stb_image.h>

#include "Core/ColladaLoader.h"
#include "Core/FBXLoader.h"
#include "Core/GLTFLoader.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/OBJLoader.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Profiler.h"
#include "Rendering/RenderAssetStoreBindings.h"
#include "Rendering/RenderEngine.h"

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
    {
        PROFILESTACK("Models");

        const std::vector<bool> state = m_models.ToStateVector();
        TLockArray<RenderAsset> a = m_models.ToLockArray();
        const uint32_t size = (uint32_t)state.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset asset = a[i];
            if (asset.InternalAddress == -1)
            {
                continue;
            }

            if (asset.Flags & 0b1 << RenderAsset::MarkBit)
            {
                asset.DeReq = 0;
                asset.Flags &= ~(0b1 << RenderAsset::MarkBit);
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

            a[i] = asset;
        }
    }
    
    {
        PROFILESTACK("Textures");

        const std::vector<bool> state = m_textures.ToStateVector();
        TLockArray<RenderAsset> a = m_textures.ToLockArray();
        const uint32_t size = (uint32_t)state.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset asset = a[i];
            if (asset.InternalAddress == -1)
            {
                continue;
            }

            if (asset.Flags & 0b1 << RenderAsset::MarkBit)
            {
                asset.DeReq = 0;
                asset.Flags &= ~(0b1 << RenderAsset::MarkBit);
            }
            else 
            {
                ++asset.DeReq;
            }

            if (asset.DeReq > RenderAssetStore::DeReqCount)
            {
                // m_renderEngine->DestroyTexture(asset.InternalAddress);
                // asset.InternalAddress = -1;
            }

            a[i] = asset;
        }
    }
}
void RenderAssetStore::Flush()
{
    {   
        const std::vector<bool> state = m_models.ToStateVector();
        TLockArray<RenderAsset> a = m_models.ToLockArray();
        const uint32_t size = (uint32_t)state.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset asset = a[i];
            if (asset.InternalAddress == -1)
            {
                continue;
            }

            m_renderEngine->DestroyModel(asset.InternalAddress);
            asset.InternalAddress = -1;

            a[i] = asset;
        }
    }
    
    {
        const std::vector<bool> state = m_textures.ToStateVector();
        TLockArray<RenderAsset> a = m_textures.ToLockArray();
        const uint32_t size = (uint32_t)state.size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (!state[i])
            {
                continue;
            }

            RenderAsset asset = a[i];
            if (asset.InternalAddress == -1)
            {
                continue;
            }

            m_renderEngine->DestroyTexture(asset.InternalAddress);
            asset.InternalAddress = -1;

            a[i] = asset;
        }
    }
}

uint32_t RenderAssetStore::LoadModel(const std::filesystem::path& a_path)
{
    RenderAsset asset;
    asset.Path = a_path.string();
    asset.InternalAddress = -1;
    asset.DeReq = 0;
    asset.Flags = 0;

    return m_models.PushVal(asset);
}
uint32_t RenderAssetStore::LoadSkinnedModel(const std::filesystem::path& a_path)
{
    RenderAsset asset;
    asset.Path = a_path.string();
    asset.InternalAddress = -1;
    asset.DeReq = 0;
    asset.Flags = 0b1 << RenderAsset::SkinnedBit;

    return m_models.PushVal(asset);
}

void RenderAssetStore::DestroyModel(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_models.Size(), "DestroyModel out of bounds");
    ICARIAN_ASSERT_MSG(m_models.Exists(a_addr), "DestroyModel already destroyed");

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
    ICARIAN_ASSERT_MSG(a_addr < m_models.Size(), "GetModel out of bounds");
    ICARIAN_ASSERT_MSG(m_models.Exists(a_addr), "GetModel already destroyed");

    TLockArray<RenderAsset> a = m_models.ToLockArray();

    RenderAsset asset = a[a_addr];
    if (asset.InternalAddress == -1)
    {
        const std::filesystem::path path = asset.Path;
        const std::filesystem::path ext = path.extension();

        const bool isSkinned = asset.Flags & 0b1 << RenderAsset::SkinnedBit;
        if (isSkinned)
        {
            constexpr uint16_t VertexStride = sizeof(SkinnedVertex);

            std::vector<SkinnedVertex> vertices;
            std::vector<uint32_t> indices;
            float radius;

            if (ext == ".dae")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load skinned file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::ColladaLoader_LoadSkinnedData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to parse skinned file: " + path.string());
                }
            }
            else if (ext == ".fbx")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load skinned file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::FBXLoader_LoadSkinnedData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to parse skinned file: " + path.string());
                }
            }
            else if (ext == ".glb" || ext == ".gltf")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load skinned file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::GLTFLoader_LoadSkinnedData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to parse skinned file: " + path.string());
                }
            }
            else 
            {
                IERROR("GetModel invalid file extension: " + path.string());
            }
        }
        else
        {
            constexpr uint16_t VertexStride = sizeof(Vertex);

            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            float radius;

            if (ext == ".obj")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::OBJLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }
            }
            else if (ext == ".dae")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::ColladaLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }
            }
            else if (ext == ".fbx")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::FBXLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }
            }
            else if (ext == ".glb" || ext == ".gltf")
            {
                FileHandle* handle = FileCache::LoadFile(path);
                if (handle == nullptr)
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }

                IDEFER(delete handle);

                const uint64_t size = handle->GetSize();
                char* data = new char[size];
                IDEFER(delete[] data);

                handle->Read(data, size);

                if (IcarianCore::GLTFLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
                {
                    asset.InternalAddress = m_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
                }
                else
                {
                    IERROR("GetModel failed to load file: " + path.string());
                }
            }
            else
            {
                IERROR("GetModel invalid file extension: " + path.string());
            }
        }
    }

    asset.DeReq = 0;
    asset.Flags |= 0b1 << RenderAsset::MarkBit;

    a[a_addr] = asset;

    return asset.InternalAddress;
}

uint32_t RenderAssetStore::LoadTexture(const std::filesystem::path& a_path)
{
    RenderAsset asset;
    asset.Path = a_path.string();
    asset.InternalAddress = -1;
    asset.DeReq = 0;
    asset.Flags = 0;

    return m_textures.PushVal(asset);
}
void RenderAssetStore::DestroyTexture(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_textures.Size(), "DestroyTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_textures.Exists(a_addr), "DestroyTexture already destroyed");

    const RenderAsset asset = m_textures[a_addr];
    IDEFER(
    if (asset.InternalAddress != -1)
    {
        m_renderEngine->DestroyTexture(asset.InternalAddress);
    });

    m_textures.Erase(a_addr);
}
uint32_t RenderAssetStore::GetTexture(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < m_textures.Size(), "GetTexture out of bounds");
    ICARIAN_ASSERT_MSG(m_textures.Exists(a_addr), "GetTexture already destroyed");

    TLockArray<RenderAsset> a = m_textures.ToLockArray();

    RenderAsset asset = a[a_addr];
    if (asset.InternalAddress == -1)
    {
        const std::filesystem::path path = asset.Path;
        const std::filesystem::path ext = path.extension();

        if (ext == ".png")
        {
            FileHandle* handle = FileCache::LoadFile(path);
            if (handle == nullptr)
            {
                IERROR("GetTexture failed to load file: " + path.string());
            }

            IDEFER(delete handle);

            const uint64_t size = handle->GetSize();
            stbi_uc* data = new stbi_uc[size];
            IDEFER(delete[] data);

            handle->Read(data, size);

	    	int width;
	    	int height;
	    	int channels;
	    	stbi_uc* pixels = stbi_load_from_memory(data, (int)size, &width, &height, &channels, STBI_rgb_alpha);
	        if (pixels != nullptr)
	    	{
                IDEFER(stbi_image_free(pixels));

	    		asset.InternalAddress = m_renderEngine->GenerateTexture((uint32_t)width, (uint32_t)height, TextureFormat_RGBA, pixels);
            }
            else
            {
                IERROR("GetTexture failed to parse file: " + path.string());
            }
        }
        else if (ext == ".ktx2")
        {
            FileHandle* handle = FileCache::LoadFile(path);
            if (handle == nullptr)
            {
                IERROR("GetTexture failed to load file: " + path.string());
            }

            IDEFER(delete handle);

            const uint64_t size = handle->GetSize();
            ktx_uint8_t* data = new ktx_uint8_t[size];
            IDEFER(delete[] data);

            handle->Read(data, size);

            ktxTexture2* texture;
            if (ktxTexture2_CreateFromMemory(data, (ktx_size_t)size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture) == KTX_SUCCESS)
            {
                IDEFER(ktxTexture_Destroy((ktxTexture*)texture));

                if (ktxTexture2_NeedsTranscoding(texture))
                {
                    ICARIAN_ASSERT_R(ktxTexture2_TranscodeBasis(texture, KTX_TTF_BC3_RGBA, 0) == KTX_SUCCESS);
                }

                const uint32_t levels = (uint32_t)texture->numLevels;
                uint64_t* offsets = new uint64_t[levels];
                IDEFER(delete[] offsets);

                for (uint32_t i = 0; i < levels; ++i)
                {
                    ktx_size_t off;
                    ICARIAN_ASSERT_R(ktxTexture_GetImageOffset((ktxTexture*)texture, i, 0, 0, &off) == KTX_SUCCESS);

                    offsets[i] = (uint64_t)off;
                }

                asset.InternalAddress = m_renderEngine->GenerateTextureMipMapped((uint32_t)texture->baseWidth, (uint32_t)texture->baseHeight, levels, offsets, TextureFormat_BC3, texture->pData, (uint64_t)texture->dataSize);
            }
            else
            {
                IERROR("GetTexture failed to parse file: " + path.string());
            }
        }
        else 
        {
            IERROR("GetTexture invalid file extension: " + path.string());
        }
    }

    asset.DeReq = 0;
    asset.Flags |= 0b1 << RenderAsset::MarkBit;

    a[a_addr] = asset;

    return asset.InternalAddress;
}