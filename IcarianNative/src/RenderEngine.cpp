// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Rendering/RenderEngine.h"

#include "Application.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Config.h"
#include "Core/DataTypes/Allocators/MallocAllocator.h"
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
#include "glm/common.hpp"

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

    const e_RenderingEngine backendEngine = m_config->GetRenderingEngine();
    switch (backendEngine)
    {
    case RenderingEngine_Null:
    {
        m_backend = IcarianCore::MallocAllocator::Instance->Create<NullRenderEngineBackend>(this);

        break;
    }
    case RenderingEngine_Vulkan:
    {
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
        m_backend = IcarianCore::MallocAllocator::Instance->Create<VulkanRenderEngineBackend>(this);
#else
        IcarianError("Vulkan is not enabled");
#endif

        break;
    }
    default:
    {
        IERROR("Failed to create RenderEngine");

        break;
    }
    }

    m_join = true;

    m_assets = IcarianCore::MallocAllocator::Instance->Create<RenderAssetStore>(this);
}
RenderEngine::~RenderEngine()
{
    TRACE("Destroying Rendering");

    Stop();

    m_assets->Flush();

    // Ensure all queued objects are freed otherwise get a double free when freed by backend doing free checks
    DeletionQueue::ClearQueue(DeletionIndex_Render);

    spirv_destroy();

    IcarianCore::MallocAllocator::Instance->Destroy(m_backend);
    IcarianCore::MallocAllocator::Instance->Destroy(m_assets);

    IcarianCore::MallocAllocator::Instance->Destroy(m_frameUpdateFunction);
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

    const Application* app = m_window->GetApplication();

    double pastDeltas[PastDeltaCount];
    double targetFrameTime = 1 / 60.0;

    uint32_t pastDeltaCount = 0;
    uint32_t pastDeltaIndex = 0;

    while (!m_shutdown)
    {
        Profiler::Start("Render Thread");
        IDEFER(Profiler::Stop());

        {
            PROFILESTACK("Update");
            const float timeScale = app->GetTimeScale();

            double delta = 0.0;

            {
                PROFILESTACK("Timing");

                {
                    PROFILESTACK("DynamicCal");

                    // We are a generic engine so we do not have much control over frametimes
                    // To get around that we lock the engine to a target FPS based off historic info
                    // This should not have worked as well as it did but turned a spiky as fuck graph flat
                    // TODO: Probably want to account for GPU frametimes down the line
                    const std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
                    const double curDelta = std::chrono::duration<double>(time - prevTime).count();

                    pastDeltas[pastDeltaIndex] = curDelta;

                    if (pastDeltaCount <= PastDeltaCount)
                    {
                        ++pastDeltaCount;
                    }
                    else
                    {
                        double max = std::numeric_limits<double>::min();
                        double min = std::numeric_limits<double>::max();

                        for (double d : pastDeltas)
                        {
                            max = glm::max(d, max);
                            min = glm::min(d, min);
                        }

                        const double diff = max - min;
                        // Sit just below the max frame time as we want to keep frametimes stable
                        const double target = max - diff * 0.1;
                        const double change = (target - targetFrameTime) * 0.5;
                        const double changeAbs = glm::abs(change);
                        const double changeSign = glm::sign(change);

                        const double maxChange = glm::min(changeAbs, (double)MaxFrameTimeAdjustment);

                        targetFrameTime += maxChange * changeSign;
                        targetFrameTime = glm::min(targetFrameTime, (double)MaxFrameTime);
                    }

                    if (m_window->IsHeadless())
                    {
                        const HeadlessAppWindow* headless = (HeadlessAppWindow*)m_window;

                        if (headless->IsRemote())
                        {
                            targetFrameTime = glm::max(targetFrameTime, (double)RemoteFrameTime);
                        }
                    }

                    // Above 500 FPS frame times get too variable so unless the user disables the cap just cap it
                    // Also no reason to waste system resources at this point
                    if (!m_config->IsFPSUnlocked())
                    {
                        targetFrameTime = glm::max(targetFrameTime, (double)MinFrameTime);
                    }

                    pastDeltaIndex = (pastDeltaIndex + 1) % PastDeltaCount;
                }

                {
                    PROFILESTACK("Wait");

                    while (true)
                    {
                        const std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
                        delta = std::chrono::duration<double>(time - prevTime).count();

                        if (delta >= targetFrameTime || m_shutdown)
                        {
                            prevTime = time;

                            break;
                        }

                        // Want to do a last mile busy wait but can sleep until the last mile
                        // We want to do sleep over yield as yield will only put us to the back of the ready queue
                        // Sleep will put us in the platform blocking thread queue until the duration has passed then put us to the back of the ready queue
                        // We want to stop sleeping before the target time as we may take a while to move from the ready queue to running
                        // All this assumes a platform with a decent scheduler and implements it the way mentioned
                        // And I swear if I have to fight the Windows scheduler 1 more time
                        // Yes I am aware some platforms have a realtime scheduler this has not caused issues not using a realtime thread so far and just doing
                        // clever timing tricks that account for delays
                        // TODO: Likely need to finetune the buffer time for busy waiting
                        const double sleepTime = targetFrameTime - delta - (SleepMillisecondBuffer / 1000.0);
                        if (sleepTime > 0.0)
                        {
                            PROFILESTACK("Sleep");

                            const std::chrono::duration<double> time = std::chrono::duration<double>(sleepTime);
                            std::this_thread::sleep_for(time);
                        }
                        else
                        {
                            PROFILESTACK("Busy Wait");

                            // Timers are not infinite resolution so just wait in place for a bit
                            // There is a small quirk on some but not all POSIX platforms that if you infinite poll with no gap between polls it will never update
                            // We do this because sometimes timers can be implemented as a poll to a hardware timer
                            // Much hair pulling in IPC code to discover that quirk hence the 1ms despite supporting 0ms in docs on polls
                            // Why do I have to know implementation details and interactions between CPUs/libc/Kernels to fix bugs
                            // Decentralization strikes once again!~
                            volatile int wait = 0;
                            while (wait++ < 16) { };
                        }
                    }
                }

                timePassed += delta;
            }

            const double scaledDelta = delta * timeScale;

            {
                PROFILESTACK("Asset Store");

                m_assets->Update();
            }

            {
                PROFILESTACK("Animators");

                AnimationController::UpdateAnimators(AnimationUpdateMode_FrameUpdate, (float)scaledDelta);
            }

            {
                PROFILESTACK("Frame Update");

                void* args[] =
                {
                    &delta,
                    &timePassed
                };

                m_frameUpdateFunction->Exec(args);
            }

            m_backend->Update(scaledDelta, timePassed);

            {
                PROFILESTACK("Deletion Queue");

                DeletionQueue::Flush(DeletionIndex_Render);
            }
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

uint32_t RenderEngine::GenerateMesh
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_meshletVertices,
    uint32_t a_meshletVertexCount,
    const uint8_t* a_meshletTriangles,
    uint32_t a_meshletTriangleCount,
    const IcarianCore::ShaderMeshletBuffer* a_meshlets,
    uint32_t a_meshletCount,
    float a_radius
)
{
    return m_backend->GenerateMesh
    (
        a_vertices,
        a_vertexCount,
        a_vertexStride,
        a_meshletVertices,
        a_meshletVertexCount,
        a_meshletTriangles,
        a_meshletTriangleCount,
        a_meshlets,
        a_meshletCount,
        a_radius
    );
}
void RenderEngine::DestroyMesh(uint32_t a_addr) const
{
    m_backend->DestroyMesh(a_addr);
}

uint32_t RenderEngine::GenerateModel
(
    const void* a_vertices,
    uint32_t a_vertexCount,
    uint16_t a_vertexStride,
    const uint32_t* a_indices,
    uint32_t a_indexCount,
    float a_radius
) const
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
uint32_t RenderEngine::GenerateTextureMipMapped
(
    uint32_t a_width,
    uint32_t a_height,
    uint32_t a_levels,
    uint64_t* a_offsets,
    e_TextureFormat a_format,
    const void* a_data,
    uint64_t a_dataSize
) const
{
    return m_backend->GenerateTextureMipMapped(a_width, a_height, a_levels, a_offsets, a_format, a_data, a_dataSize);
}
void RenderEngine::DestroyTexture(uint32_t a_addr) const
{
    m_backend->DestroyTexture(a_addr);
}

uint32_t RenderEngine::GenerateTextureSampler
(
    uint32_t a_textureAddr,
    e_TextureMode a_textureMode,
    e_TextureFilter a_filterMode,
    e_TextureAddress a_addressMode,
    uint32_t a_slot
) const
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
