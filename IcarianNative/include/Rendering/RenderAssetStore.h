// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "DataTypes/TNCArray.h"
#include "DataTypes/TStatic.h"

class Font;
class RenderEngine;
class RenderAssetStoreBindings;
class StackAllocator;

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

struct RenderAssetScratchAllocator
{
    volatile uint32_t Count;
    StackAllocator* Allocator;
};

#define ISRENDERASSETSTOREADDR(assetAddr) ((assetAddr) & 0b1 << RenderAssetStore::RenderAssetStoreBit)
#define FROMRENDERSTOREADDR(assetAddr) ((assetAddr) & ~(0b1 << RenderAssetStore::RenderAssetStoreBit))
#define TORENDERSTOREADDR(assetAddr) ((assetAddr) | 0b1 << RenderAssetStore::RenderAssetStoreBit)

#include "EngineModelInteropStructures.h"

class RenderAssetStore
{
public:
    static constexpr uint32_t RenderAssetStoreBit = 30;

private:
    struct ClassData
    {
        Array<RenderAssetScratchAllocator> StackAllocators;

        RenderEngine*                      Renderer;
        RenderAssetStoreBindings*          Bindings;

        TNCArray<RenderAsset>              Meshes;
        TNCArray<RenderAsset>              Models;
        TNCArray<RenderAsset>              Textures;
        TNCArray<Font*>                    Fonts;

        uint32_t                           ScratchIndex;
    };

    constexpr static uint64_t BlockAllocatorSize = 32 << 20;
    constexpr static uint64_t ScratchAllocatorSize = 64 << 20;

    friend class RenderAssetStoreBindings;

    static constexpr uint16_t DeReqCount = 20;

    Allocator*        m_blockAllocator;

    ClassData*        m_data;

    SpinLock          m_scratchLock;

    uint32_t LoadSkinnedModelFile(RenderEngine* a_renderEngine, uint8_t a_data, const std::string_view& a_path);
    uint32_t LoadMeshData(const std::string_view& a_path, uint8_t a_index);

protected:

public:
    RenderAssetStore(RenderEngine* a_renderEngine);
    ~RenderAssetStore();

    void Update();
    void Flush();

    inline Font* GetFont(uint32_t a_addr)
    {
        return m_data->Fonts[a_addr];
    }

    [[nodiscard]] uint32_t LoadMesh(const std::string_view& a_path, uint8_t a_index);
    void DestroyMesh(uint32_t a_addr);
    uint32_t GetMesh(uint32_t a_addr);

    bool LoadModelData(const std::string_view& a_path, uint8_t a_data, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices, float* a_radius);
    [[nodiscard]] uint32_t LoadModel(const std::string_view& a_path, uint8_t a_index);
    [[nodiscard]] uint32_t LoadSkinnedModel(const std::string_view& a_path, uint8_t a_index);
    void DestroyModel(uint32_t a_addr);
    uint32_t GetModel(uint32_t a_addr);

    [[nodiscard]] uint32_t LoadTexture(const std::string_view& a_path);
    void DestroyTexture(uint32_t a_addr);
    uint32_t GetTexture(uint32_t a_addr);

    uint32_t GetScratchAllocatorIndex();
};

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
