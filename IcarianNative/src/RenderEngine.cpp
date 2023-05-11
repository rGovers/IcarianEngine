#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/SpirvTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

RenderEngine::RenderEngine(RuntimeManager* a_runtime, ObjectManager* a_objectManager, AppWindow* a_window, Config* a_config)
{
    TRACE("Initializing Rendering");
    m_config = a_config;

    m_objectManager = a_objectManager;

    m_window = a_window;

    spirv_init();

    assert(m_config->GetRenderingEngine() != RenderingEngine_Null);

    switch (m_config->GetRenderingEngine())
    {
    case RenderingEngine_Vulkan:
    {
        m_backend = new VulkanRenderEngineBackend(a_runtime, this);

        break;
    }
    default:
    {
        Logger::Error("Failed to create RenderEngine");

        assert(0);

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
    std::chrono::time_point prevTime = std::chrono::high_resolution_clock::now();

    while (!m_shutdown)
    {
        Profiler::Start("Render Thread");
        
        {
            PROFILESTACK("Update");

            const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

            const double delta = std::chrono::duration<double>(time - prevTime).count();
            m_time += delta;

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
