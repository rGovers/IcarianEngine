#pragma once

#include <string_view>
#include <thread>

#include "Flare/TextureSampler.h"

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
class RuntimeManager;

class RenderEngine
{
private:
    friend class VulkanRenderEngineBackend;

    double               m_time;

    // If not volatile GCC may optimize away the stop function
    // Program will not terminate if stop is optimized away
    volatile bool        m_shutdown;
    volatile bool        m_join;
    std::thread          m_thread;

    Config*              m_config;

    ObjectManager*       m_objectManager;

    RenderEngineBackend* m_backend;

    AppWindow*           m_window;

    RuntimeManager*      m_runtime;

    void Update(double a_delta, double a_time);
    void Run();
protected:

public:
    RenderEngine(RuntimeManager* a_runtime, ObjectManager* a_objectManager, AppWindow* a_window, Config* a_config);
    ~RenderEngine();

    void Start();
    void Stop();

    uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data) const;
    void DestroyTexture(uint32_t a_addr) const;

    uint32_t GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot = 0) const;
    void DestroyTextureSampler(uint32_t a_addr) const;

    Font* GetFont(uint32_t a_addr) const;

    inline ObjectManager* GetObjectManager() const
    {
        return m_objectManager;
    }
};