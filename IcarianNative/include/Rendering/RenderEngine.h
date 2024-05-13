#pragma once

#include <thread>

#include "EngineTextureSamplerInteropStructures.h"

#include "Rendering/TextureData.h"

enum e_RenderingEngine
{
    RenderingEngine_Null,
    RenderingEngine_Vulkan
};

class AppWindow;
class Config;
class Font;
class ObjectManager;
class RenderAssetStore;
class RenderEngineBackend;
class RuntimeFunction;
class RuntimeManager;

class RenderEngine
{
private:
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    friend class VulkanRenderEngineBackend;
#endif
    std::thread          m_thread;

    Config*              m_config;

    RenderAssetStore*    m_assets;
    RenderEngineBackend* m_backend;

    AppWindow*           m_window;

    RuntimeFunction*     m_frameUpdateFunction;

    // If not volatile GCC may optimize away the stop function
    // Program will not terminate if stop is optimized away
    volatile bool        m_shutdown;
    volatile bool        m_join;

    void Update(double a_delta, double a_time);
    void Run();
protected:

public:
    RenderEngine(AppWindow* a_window, Config* a_config);
    ~RenderEngine();

    void Start();
    void Stop();

    inline RenderAssetStore* GetRenderAssetStore() const
    {
        return m_assets;
    }

    uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius) const;
    void DestroyModel(uint32_t a_addr) const;

    uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data) const;
    uint32_t GenerateTextureMipMapped(uint32_t a_width, uint32_t a_height, uint32_t a_levels, uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize) const;
    void DestroyTexture(uint32_t a_addr) const;

    uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0) const;
    void DestroyTextureSampler(uint32_t a_addr) const;

    Font* GetFont(uint32_t a_addr) const;
};