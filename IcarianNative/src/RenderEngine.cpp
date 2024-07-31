#include "Rendering/RenderEngine.h"


#include "Config.h"
#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "DeletionQueue.h"
#include "Profiler.h"
#include "Rendering/AnimationController.h"
#include "Rendering/Null/NullRenderEngineBackend.h"
#include "Rendering/RenderAssetStore.h"
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

    m_assets = new RenderAssetStore(this);
}
RenderEngine::~RenderEngine()
{
    TRACE("Destroying Rendering");

    Stop();

    m_assets->Flush();

    // Ensure all queued objects are freed otherwise get a double free when freed by backend doing free checks
    DeletionQueue::ClearQueue(DeletionIndex_Render);

    spirv_destroy();

    delete m_backend;
    delete m_assets;

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
    while (!m_join) 
    {
        std::this_thread::yield();
    }
    m_thread.join();

    DeletionQueue::ClearQueue(DeletionIndex_Render);
}

void RenderEngine::Run()
{
    RuntimeManager::AttachThread();

    double timePassed = 0.0;
    std::chrono::time_point prevTime = std::chrono::high_resolution_clock::now();

    while (!m_shutdown)
    {
        Profiler::Start("Render Thread");
        IDEFER(Profiler::Stop());
        
        {
            PROFILESTACK("Update");

            const std::chrono::time_point time = std::chrono::high_resolution_clock::now();

            double delta = std::chrono::duration<double>(time - prevTime).count();
            timePassed += delta;

            {
                PROFILESTACK("Asset Store");

                m_assets->Update();
            }

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

            {
                PROFILESTACK("Deletion Queue");

                DeletionQueue::Flush(DeletionIndex_Render);
            }

            prevTime = time;
        }   
    }
    
    m_join = true;
    TRACE("Render Thread joining");
}
void RenderEngine::Update(double a_delta, double a_time)
{
    m_backend->Update(a_delta, a_time);
}

e_RenderDeviceType RenderEngine::GetDeviceType() const
{
    return m_backend->GetDeviceType();
}

uint64_t RenderEngine::GetUsedDeviceMemory() const
{
    return m_backend->GetUsedDeviceMemory();
}
uint64_t RenderEngine::GetTotalDeviceMemory() const
{
    return m_backend->GetTotalDeviceMemory();
}

uint32_t RenderEngine::GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius) const
{
    return m_backend->GenerateModel(a_vertices, a_vertexCount, a_vertexStride, a_indices, a_indexCount, a_radius);
}
void RenderEngine::DestroyModel(uint32_t a_addr) const
{
    m_backend->DestroyModel(a_addr);
}

uint32_t RenderEngine::GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data) const
{
    return m_backend->GenerateTexture(a_width, a_height, a_format, a_data);
}
uint32_t RenderEngine::GenerateTextureMipMapped(uint32_t a_width, uint32_t a_height, uint32_t a_levels, uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize) const
{
    return m_backend->GenerateTextureMipMapped(a_width, a_height, a_levels, a_offsets, a_format, a_data, a_dataSize);
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
    return m_assets->GetFont(a_addr);
}