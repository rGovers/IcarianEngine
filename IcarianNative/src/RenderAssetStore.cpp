#include "Rendering/RenderAssetStore.h"

#include <ktx.h>
#include <stb_image.h>

#include "Core/Bitfield.h"
#include "Core/ColladaLoader.h"
#include "Core/FBXLoader.h"
#include "Core/GLTFLoader.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Core/OBJLoader.h"
#include "Core/StringUtils.h"
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
        }
    }
    
    {
        PROFILESTACK("Textures");

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

uint32_t RenderAssetStore::LoadModel(const std::filesystem::path& a_path)
{
    FileCache::PreLoad(a_path);

    const RenderAsset asset =
    {
        .Path = a_path.string(),
        .InternalAddress = uint32_t(-1),
    };

    return m_models.PushVal(asset);
}
uint32_t RenderAssetStore::LoadSkinnedModel(const std::filesystem::path& a_path)
{
    FileCache::PreLoad(a_path);

    const RenderAsset asset =
    {
        .Path = a_path.string(),
        .InternalAddress = uint32_t(-1),
        .Flags = 0b1 << RenderAsset::SkinnedBit
    };

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

static uint32_t LoadSkinnedModelFile(RenderEngine* a_renderEngine, const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    constexpr uint16_t VertexStride = sizeof(SkinnedVertex);

    std::vector<SkinnedVertex> vertices;
    std::vector<uint32_t> indices;
    float radius;

    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".dae"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load skinned file: " + a_path.string());

            break;
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::ColladaLoader_LoadSkinnedData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to parse skinned file: " + a_path.string());

        break;
    }
    case StringHash<uint32_t>(".fbx"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load skinned file: " + a_path.string());

            break;
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::FBXLoader_LoadSkinnedData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to parse skinned file: " + a_path.string());

        break;
    }
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load skinned file: " + a_path.string());

            break;
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::GLTFLoader_LoadSkinnedData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to parse skinned file: " + a_path.string());

        break;
    }
    default:
    {
        IERROR("GetModel invalid file extension: " + a_path.string());

        break;
    }
    }

    return -1;
}
static uint32_t LoadBaseModelFile(RenderEngine* a_renderEngine, const std::filesystem::path& a_path)
{
    const std::filesystem::path ext = a_path.extension();
    const std::string extStr = ext.string();

    constexpr uint16_t VertexStride = sizeof(Vertex);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    float radius;

    switch (StringHash<uint32_t>(extStr.c_str())) 
    {
    case StringHash<uint32_t>(".obj"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load file: " + a_path.string());
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::OBJLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to load file: " + a_path.string());

        break;
    }
    case StringHash<uint32_t>(".dae"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load file: " + a_path.string());
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::ColladaLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to load file: " + a_path.string());

        break;
    }
    case StringHash<uint32_t>(".fbx"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load file: " + a_path.string());
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::FBXLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to load file: " + a_path.string());

        break;
    }
    case StringHash<uint32_t>(".glb"):
    case StringHash<uint32_t>(".gltf"):
    {
        FileHandle* handle = FileCache::LoadFile(a_path);
        if (handle == nullptr)
        {
            IERROR("GetModel failed to load file: " + a_path.string());
        }

        IDEFER(delete handle);

        const uint64_t size = handle->GetSize();
        char* data = new char[size];
        IDEFER(delete[] data);

        handle->Read(data, size);

        if (IcarianCore::GLTFLoader_LoadData(data, (uint32_t)size, &vertices, &indices, &radius))
        {
            return a_renderEngine->GenerateModel(vertices.data(), (uint32_t)vertices.size(), VertexStride, indices.data(), (uint32_t)indices.size(), radius);
        }

        IERROR("GetModel failed to load file: " + a_path.string());
        
        break;
    }
    default:
    {
        IERROR("GetModel invalid file extension: " + a_path.string());

        break;
    }
    }

    return -1;
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
            asset.InternalAddress = LoadSkinnedModelFile(m_renderEngine, path);
        }
        else
        {
            asset.InternalAddress = LoadBaseModelFile(m_renderEngine, path);
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