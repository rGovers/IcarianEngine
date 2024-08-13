// Icarian Engine - C# Game Engine
// 
// License at end of file.

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
    uint8_t Data;
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

    uint32_t LoadModel(const std::filesystem::path& a_path, uint32_t a_index);
    uint32_t LoadSkinnedModel(const std::filesystem::path& a_path, uint32_t a_index);
    void DestroyModel(uint32_t a_addr);
    uint32_t GetModel(uint32_t a_addr);

    uint32_t LoadTexture(const std::filesystem::path& a_path);
    void DestroyTexture(uint32_t a_addr);
    uint32_t GetTexture(uint32_t a_addr);
};

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