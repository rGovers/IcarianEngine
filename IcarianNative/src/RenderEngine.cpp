#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
#include "DeletionQueue.h"
#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/Null/NullRenderEngineBackend.h"
#include "Rendering/SPIRVTools.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#endif

RenderEngine::RenderEngine(AppWindow* a_window, Config* a_config)
{
    TRACE("Initializing Rendering");
    m_config = a_config;

    m_frameUpdateFunction = RuntimeManager::GetFunction("IcarianEngine", "Program", ":FrameUpdate(double,double)");

    m_window = a_window;

    spirv_init();

    switch (m_config->GetRenderingEngine())
    {
    case RenderingEngine_Null:
    {
        m_backend = new NullRenderEngineBackend(this);

        break;
    }
    case RenderingEngine_Vulkan:
    {
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
        m_backend = new VulkanRenderEngineBackend(this);
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
    TRACE("Destroying Rendering");

    Stop();

    spirv_destroy();

    delete m_backend;

    delete m_frameUpdateFunction;
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
    RuntimeManager::AttachThread();

    std::chrono::time_point prevTime = std::chrono::high_resolution_clock::now();

    double timePassed = 0.0;

    while (!m_shutdown)
    {
        Profiler::Start("Render Thread");
        
        {
            PROFILESTACK("Update");

            const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

            double delta = std::chrono::duration<double>(time - prevTime).count();
            timePassed += delta;

            {
                PROFILESTACK("Animators");
                
                AnimationController::UpdateAnimators(AnimationUpdateMode_FrameUpdate, (float)delta);
            }

            void* args[] =
            {
                &delta,
                &timePassed
            };

            {
                PROFILESTACK("Frame Update");
                
                m_frameUpdateFunction->Exec(args);
            }

            m_backend->Update(delta, timePassed);

            prevTime = time;
        }   
        
        DeletionQueue::Flush(DeletionIndex_Render);

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

uint32_t RenderEngine::GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot) const
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