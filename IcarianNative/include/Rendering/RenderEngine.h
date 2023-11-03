#pragma once

#include <string_view>
#include <thread>

#include "EngineTextureSamplerInteropStructures.h"

enum e_RenderingEngine
{
    RenderingEngine_Null,
    RenderingEngine_Vulkan
};

class AppWindow;
class Config;
class Font;
class ObjectManager;
class RenderEngineBackend;
class RuntimeFunction;
class RuntimeManager;

class RenderEngine
{
private:
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    friend class VulkanRenderEngineBackend;
#endif

    // If not volatile GCC may optimize away the stop function
    // Program will not terminate if stop is optimized away
    volatile bool        m_shutdown;
    volatile bool        m_join;
    std::thread          m_thread;

    Config*              m_config;

    RenderEngineBackend* m_backend;

    AppWindow*           m_window;

    RuntimeFunction*     m_frameUpdateFunction;

    void Update(double a_delta, double a_time);
    void Run();
protected:

public:
    RenderEngine(AppWindow* a_window, Config* a_config);
    ~RenderEngine();

    void Start();
    void Stop();

    uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data) const;
    void DestroyTexture(uint32_t a_addr) const;

    uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0) const;
    void DestroyTextureSampler(uint32_t a_addr) const;

    Font* GetFont(uint32_t a_addr) const;
};