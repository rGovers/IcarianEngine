#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/Null/NullRenderEngineBackend.h"
#include "Rendering/SpirvTools.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#endif

RenderEngine::RenderEngine(RuntimeManager* a_runtime, ObjectManager* a_objectManager, AppWindow* a_window, Config* a_config)
{
    TRACE("Initializing Rendering");
    m_config = a_config;

    m_runtime = a_runtime;

    m_objectManager = a_objectManager;

    m_window = a_window;

    spirv_init();

    switch (m_config->GetRenderingEngine())
    {
    case RenderingEngine_Null:
    {
        m_backend = new NullRenderEngineBackend(a_runtime, this);

        break;
    }
    case RenderingEngine_Vulkan:
    {
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
        m_backend = new VulkanRenderEngineBackend(a_runtime, this);
#else
        ICARIAN_ASSERT_MSG_R(0, "Vulkan is not enabled");
#endif

        break;
    }
    default:
    {
        ICARIAN_ASSERT_MSG_R(0, "Failed to create RenderEngine");

        break;
    }
    }

    m_join = true;
}
RenderEngine::~RenderEngine()
{
    Stop();

    spirv_destroy();

    delete m_backend;
}

void RenderEngine::Start()
{
    TRACE("Starting Render Thread");
    m_shutdown = false;
    m_join = false;
    m_thread = std::thread(std::bind(&RenderEngine::Run, this));
}
void RenderEngine::Stop()
{
    if (m_join)
    {
        return;
    }

    TRACE("Stopping Render Thread");
    m_shutdown = true;
    while (!m_join) { }
    m_thread.join();
}

void RenderEngine::Run()
{
    m_runtime->AttachThread();

    std::chrono::time_point prevTime = std::chrono::high_resolution_clock::now();

    while (!m_shutdown)
    {
        Profiler::Start("Render Thread");
        
        {
            PROFILESTACK("Update");

            const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

            const double delta = std::chrono::duration<double>(time - prevTime).count();
            m_time += delta;

            {
                PROFILESTACK("Animators");
                
                AnimationController::UpdateAnimators(AnimationUpdateMode_FrameUpdate, (float)delta);
            }

            m_backend->Update(delta, m_time);

            prevTime = time;
        }   
        
        Profiler::Stop();
    }
    
    m_join = true;
    TRACE("Render Thread joining");
}
void RenderEngine::Update(double a_delta, double a_time)
{
    m_backend->Update(a_delta, a_time);
}

uint32_t RenderEngine::GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data) const
{
    return m_backend->GenerateAlphaTexture(a_width, a_height, a_data);
}
void RenderEngine::DestroyTexture(uint32_t a_addr) const
{
    m_backend->DestroyTexture(a_addr);
}

uint32_t RenderEngine::GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot) const
{
    return m_backend->GenerateTextureSampler(a_textureAddr, a_textureMode, a_filterMode, a_addressMode, a_slot);
}
void RenderEngine::DestroyTextureSampler(uint32_t a_addr) const
{
    m_backend->DestroyTextureSampler(a_addr);
}

Font* RenderEngine::GetFont(uint32_t a_addr) const
{
    return m_backend->GetFont(a_addr);
}