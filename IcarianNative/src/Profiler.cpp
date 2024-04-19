#include "Profiler.h"

#include <mutex>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

Profiler* Profiler::Instance = nullptr;
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
}
Profiler::~Profiler()
{

}

void Profiler::Init()
{
    TRACE("Initializing Profiler");
    if (Instance == nullptr)
    {
        Instance = new Profiler();
    }
}
void Profiler::Destroy()
{
    TRACE("Destroying Profiler");
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void Profiler::Start(const std::string_view& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::thread::id tID = std::this_thread::get_id();

    const std::unique_lock lock = std::unique_lock(Instance->m_mutex);

    PData data = PData();
    data.Name = std::string(a_name);

    Instance->m_data.emplace(tID, data);
#endif
}
void Profiler::Stop()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::thread::id tID = std::this_thread::get_id();

    const std::unique_lock lock = std::unique_lock(Instance->m_mutex);

    const auto iter = Instance->m_data.find(tID);
    ICARIAN_ASSERT_MSG(iter != Instance->m_data.end(), "Profiler not started on thread");
    
    if (CallbackFunc != nullptr)
    {
        (*CallbackFunc)(iter->second);
    }

    Instance->m_data.erase(iter);
#endif
}

void Profiler::StartFrame(const std::string_view& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    const std::thread::id tID = std::this_thread::get_id();

    const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

    const auto iter = Instance->m_data.find(tID);
    ICARIAN_ASSERT_MSG(iter != Instance->m_data.end(), "Profiler not started on thread");

    uint32_t stack = 0;
    if (!iter->second.Frames.empty())
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

    ProfileFrame frame;
    frame.StartTime = startTime;
    frame.Duration = 0.0;
    frame.Name = std::string(a_name);
    frame.Stack = stack;
    frame.End = false;

    iter->second.Frames.emplace_back(frame);
#endif
}
void Profiler::StopFrame()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
    const std::thread::id tID = std::this_thread::get_id();

    const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

    const auto iter = Instance->m_data.find(tID);
    ICARIAN_ASSERT_MSG(iter != Instance->m_data.end(), "Profiler not started on thread");
    ICARIAN_ASSERT_MSG(!iter->second.Frames.empty(), "Profiler Frame not created on thread");

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

    ICARIAN_ASSERT_MSG(0, "Profiler Start End Frame mismatch");
#endif
}