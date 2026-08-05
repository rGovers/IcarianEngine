// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Profiler.h"

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/ThreadGuard.h"
#include "Core/IcarianDefer.h"
#include "IcarianError.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static Profiler* Instance = nullptr;

Profiler::CPUCallbackItem* Profiler::CPUCallback = nullptr;
Profiler::GPUCallbackItem* Profiler::GPUCallback = nullptr;
Profiler::GPUETECallbackItem* Profiler::GPUETECallback = nullptr;

RUNTIME_FUNCTION(void, Profiler, StartFrame,
{
    char* str = mono_string_to_utf8(a_frameName);
    IDEFER(mono_free(str));

    Profiler::StartFrame(str);
}, MonoString* a_frameName)
RUNTIME_FUNCTION(void, Profiler, StopFrame,
{
    Profiler::StopFrame();
})

Profiler::Profiler() :
    m_data(IcarianCore::MallocAllocator::Instance)
{
    BIND_FUNCTION(IcarianEngine, Profiler, StartFrame);
    BIND_FUNCTION(IcarianEngine, Profiler, StopFrame);

    m_memoryFrame = { };
}
Profiler::~Profiler()
{

}

void Profiler::Init()
{
    TRACE("Initializing Profiler");
    if (Instance == nullptr)
    {
        Instance = IcarianCore::MallocAllocator::Instance->Create<Profiler>();
    }
}
void Profiler::Destroy()
{
    TRACE("Destroying Profiler");
    if (Instance != nullptr)
    {
        IcarianCore::MallocAllocator::Instance->Destroy(Instance);
        Instance = nullptr;
    }
}

void Profiler::Start(const char* a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    Start(IcarianCore::COWU8String(a_name, IcarianCore::MallocAllocator::Instance));
#endif
}
void Profiler::Start(const IcarianCore::COWU8String& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::thread::id tID = std::this_thread::get_id();

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(Instance->m_lock);

    const ProfilerCPUData data =
    {
        .Name = IcarianCore::COWU8String(a_name, IcarianCore::MallocAllocator::Instance),
        .Frames = IcarianCore::Array<ProfileFrame>(IcarianCore::MallocAllocator::Instance),
    };

    Instance->m_data.Push(tID, data);
#endif
}
void Profiler::Stop()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::thread::id tID = std::this_thread::get_id();

    const IcarianCore::ThreadGuard lock = IcarianCore::ThreadGuard(Instance->m_lock);

    IVERIFY(Instance->m_data.Exists(tID));
    IDEFER(Instance->m_data.Erase(tID));

    if (CPUCallback != nullptr)
    {
        const ProfilerCPUData& data = Instance->m_data[tID];

        CPUCallback->Execute(data);
    }
#endif
}

void Profiler::PushMemoryFrame(e_ProfilerMemoryFrame a_frame, uint64_t a_size)
{
    switch (a_frame)
    {
    case ProfilerMemoryFrame_CSharp:
    {
        Instance->m_memoryFrame.CSharpUsage = a_size;

        break;
    }
    case ProfilerMemoryFrame_Audio:
    {
        Instance->m_memoryFrame.AudioUsage = a_size;

        break;
    }
    case ProfilerMemoryFrame_Rendering:
    {
        Instance->m_memoryFrame.RenderingUsage = a_size;

        break;
    }
    case ProfilerMemoryFrame_Physics:
    {
        Instance->m_memoryFrame.PhysicsUsage = a_size;

        break;
    }
    case ProfilerMemoryFrame_FileCache:
    {
        Instance->m_memoryFrame.FileCacheUsage = a_size;

        break;
    }
    default:
    {
        IERROR("Invalid memory frame");

        break;
    }
    }
}
IcarianCore::MemoryUsageFrame Profiler::GetMemoryFrames()
{
    return Instance->m_memoryFrame;
}

void Profiler::StartFrame(const char* a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    StartFrame(IcarianCore::COWU8String(a_name, IcarianCore::MallocAllocator::Instance));
#endif
}
void Profiler::StartFrame(const IcarianCore::COWU8String& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    const std::thread::id tID = std::this_thread::get_id();

    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(Instance->m_lock);

    IVERIFY(Instance->m_data.Exists(tID));

    ProfilerCPUData& data = Instance->m_data[tID];

    uint32_t stack = 0;
    if (!data.Frames.Empty())
    {
        auto iIter = data.Frames.end();
        while (iIter != data.Frames.begin())
        {
            --iIter;

            if (iIter->Name == a_name)
            {
                iIter->StartTime = startTime;
                iIter->End = false;

                return;
            }

            if (!iIter->End)
            {
                stack = iIter->Stack + 1;

                break;
            }
        }
    }

    const ProfileFrame frame =
    {
        .Name = IcarianCore::COWU8String(a_name, IcarianCore::MallocAllocator::Instance),
        .Duration = 0.0,
        .StartTime = startTime,
        .Stack = stack,
        .End = false,
    };

    data.Frames.Push(frame);
#endif
}
void Profiler::StopFrame()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
    const std::thread::id tID = std::this_thread::get_id();

    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(Instance->m_lock);

    IVERIFY(Instance->m_data.Exists(tID));

    ProfilerCPUData& data = Instance->m_data[tID];
    IVERIFY(!data.Frames.Empty());

    auto iIter = data.Frames.end();
    while (iIter != data.Frames.begin())
    {
        --iIter;

        if (!iIter->End)
        {
            iIter->Duration += std::chrono::duration_cast<std::chrono::duration<double>>(endTime - iIter->StartTime).count();
            iIter->End = true;

            return;
        }
    }

    IERROR("Profiler Start End Frame mismatch");
#endif
}

void Profiler::PushGPUData(const ProfilerGPUFrameData* a_data, uint32_t a_count)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    if (a_count <= 0)
    {
        return;
    }

    if (GPUCallback != nullptr)
    {
        GPUCallback->Execute(a_data, a_count);
    }
#endif
}
void Profiler::PushGPUEndToEndTime(float a_time)
{
    if (GPUETECallback != nullptr)
    {
        GPUETECallback->Execute(a_time);
    }
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
