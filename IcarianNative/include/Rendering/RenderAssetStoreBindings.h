#pragma once

#include <cstdint>
#include <filesystem>

class RenderAssetStore;

class RenderAssetStoreBindings
{
private:
    RenderAssetStore* m_store;

protected:

public:
    RenderAssetStoreBindings(RenderAssetStore* a_store);
    ~RenderAssetStoreBindings();

    uint32_t GenerateFont(const std::filesystem::path& a_path) const;
    void DestroyFont(uint32_t a_addr) const;

    uint32_t GenerateModelFromString(uint32_t a_addr, const std::u32string_view& a_str, float a_fontSize, float a_scale, float a_depth) const;

    uint32_t GenerateModel(const std::filesystem::path& a_path, uint32_t a_index) const;
    uint32_t GenerateSkinnedModel(const std::filesystem::path& a_path, uint32_t a_index) const;

    uint32_t GenerateTexture(const std::filesystem::path& a_path) const;
};