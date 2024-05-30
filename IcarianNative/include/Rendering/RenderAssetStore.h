#pragma once

#include <cstdint>
#include <filesystem>

#include "DataTypes/TNCArray.h"

class Font;
class RenderEngine;
class RenderAssetStoreBindings;

struct RenderAsset
{
    static constexpr uint32_t MarkBit = 0;
    static constexpr uint32_t SkinnedBit = 1;

    // Paths are broken but strings work for some bloody reason
    // std::filesystem::path Path;
    std::string Path;
    uint32_t InternalAddress;
    uint16_t DeReq;
    uint8_t Flags;
};

#define ISRENDERASSETSTOREADDR(assetAddr) ((assetAddr) & 0b1 << RenderAssetStore::RenderAssetStoreBit)
#define FROMRENDERSTOREADDR(assetAddr) ((assetAddr) & ~(0b1 << RenderAssetStore::RenderAssetStoreBit))
#define TORENDERSTOREADDR(assetAddr) ((assetAddr) | 0b1 << RenderAssetStore::RenderAssetStoreBit)

class RenderAssetStore
{
public:
    static constexpr uint32_t RenderAssetStoreBit = 30;

private:
    friend class RenderAssetStoreBindings;

    static constexpr uint16_t DeReqCount = 20;

    RenderEngine*             m_renderEngine;
    RenderAssetStoreBindings* m_bindings;

    TNCArray<RenderAsset>     m_models;
    TNCArray<RenderAsset>     m_textures;
    TNCArray<Font*>           m_fonts;

protected:

public:
    RenderAssetStore(RenderEngine* a_renderEngine);
    ~RenderAssetStore();

    void Update();
    void Flush();

    inline Font* GetFont(uint32_t a_addr)
    {
        return m_fonts[a_addr];
    }

    uint32_t LoadModel(const std::filesystem::path& a_path);
    uint32_t LoadSkinnedModel(const std::filesystem::path& a_path);
    void DestroyModel(uint32_t a_addr);
    uint32_t GetModel(uint32_t a_addr);

    uint32_t LoadTexture(const std::filesystem::path& a_path);
    void DestroyTexture(uint32_t a_addr);
    uint32_t GetTexture(uint32_t a_addr);
};