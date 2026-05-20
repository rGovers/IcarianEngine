// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Profiler.h"

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "DataTypes/ThreadGuard.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static Profiler* Instance = nullptr;
Profiler::Callback* Profiler::CallbackFunc = nullptr;

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

Profiler::Profiler()
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
        Instance = MallocAllocator::Instance->Create<Profiler>();
    }
}
void Profiler::Destroy()
{
    TRACE("Destroying Profiler");
    if (Instance != nullptr)
    {
        MallocAllocator::Instance->Destroy(Instance);
        Instance = nullptr;
    }
}

void Profiler::Start(const char* a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    Start(COWU8String(a_name, MallocAllocator::Instance));
#endif
}
void Profiler::Start(const COWU8String& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::thread::id tID = std::this_thread::get_id();

    const ThreadGuard g = ThreadGuard(Instance->m_lock);

    const PData data =
    {
        .Name = COWU8String(a_name, MallocAllocator::Instance),
        .Frames = Array<ProfileFrame>(MallocAllocator::Instance),
    };

    Instance->m_data.emplace(tID, data);
#endif
}
void Profiler::Stop()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::thread::id tID = std::this_thread::get_id();

    const ThreadGuard lock = ThreadGuard(Instance->m_lock);

    const auto iter = Instance->m_data.find(tID);
    ICARIAN_ASSERT_MSG(iter != Instance->m_data.end(), "Profiler not started on thread");

    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(iter->second);
    }

    Instance->m_data.erase(iter);
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
    StartFrame(COWU8String(a_name, MallocAllocator::Instance));
#endif
}
void Profiler::StartFrame(const COWU8String& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    const std::thread::id tID = std::this_thread::get_id();

    const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

    const auto iter = Instance->m_data.find(tID);
    IVERIFY(iter != Instance->m_data.end());

    uint32_t stack = 0;
    if (!iter->second.Frames.Empty())
    {
        auto iIter = iter->second.Frames.end();
        while (iIter != iter->second.Frames.begin())
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
        .Name = COWU8String(a_name, MallocAllocator::Instance),
        .Duration = 0.0,
        .StartTime = startTime,
        .Stack = stack,
        .End = false,
    };

    iter->second.Frames.Push(frame);
#endif
}
void Profiler::StopFrame()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
    const std::thread::id tID = std::this_thread::get_id();

    const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

    const auto iter = Instance->m_data.find(tID);
    IVERIFY(iter != Instance->m_data.end());
    IVERIFY(!iter->second.Frames.Empty());

    auto iIter = iter->second.Frames.end();
    while (iIter != iter->second.Frames.begin())
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