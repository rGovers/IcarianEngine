#include "Profiler.h"

#include <cassert>
#include <mutex>

#include "Flare/IcarianDefer.h"
#include "Logger.h"
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
    for (auto iter = m_data.begin(); iter != m_data.end(); ++iter)
    {
        delete iter->second;
    }
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
    const std::unique_lock lock = std::unique_lock(Instance->m_mutex);

    const std::thread::id tID = std::this_thread::get_id();

    auto iter = Instance->m_data.find(tID);

    if (iter != Instance->m_data.end())
    {
        iter->second->Frames.clear();
    }
    else
    {
        PData* data = new PData();
        data->Name = std::string(a_name);

        Instance->m_data.emplace(tID, data);
    }
#endif
}
void Profiler::Stop()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    if (CallbackFunc != nullptr)
    {
        const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

        const std::thread::id tID = std::this_thread::get_id();

        const auto iter = Instance->m_data.find(tID);
        if (iter == Instance->m_data.end())
        {
            Logger::Error("IcarianEngine: Profiler not started on thread");

            assert(0);
        }

        (*CallbackFunc)(*iter->second);
    }
#endif
}

void Profiler::StartFrame(const std::string_view& a_name)
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

    const std::thread::id tID = std::this_thread::get_id();

    const auto iter = Instance->m_data.find(tID);
    if (iter == Instance->m_data.end())
    {
        Logger::Error("IcarianEngine: Profiler not started on thread");

        assert(0);
    }

    ProfileFrame frame;
    frame.StartTime = std::chrono::high_resolution_clock::now();
    frame.EndTime = frame.StartTime;
    frame.Name = std::string(a_name);
    frame.Stack = 0;
    frame.End = false;
    if (!iter->second->Frames.empty())
    {
        auto iIter = iter->second->Frames.end();
        while (iIter != iter->second->Frames.begin())
        {
            --iIter;

            if (!iIter->End)
            {
                frame.Stack = iIter->Stack + 1;

                break;
            }
        }
    }

    iter->second->Frames.emplace_back(frame);
#endif
}
void Profiler::StopFrame()
{
#ifdef ICARIANNATIVE_ENABLE_PROFILER
    const std::shared_lock lock = std::shared_lock(Instance->m_mutex);

    const std::thread::id tID = std::this_thread::get_id();

    const auto iter = Instance->m_data.find(tID);
    if (iter == Instance->m_data.end())
    {
        Logger::Error("IcarianEngine: Profiler not started on thread");

        assert(0);
    }

    if (iter->second->Frames.empty())
    {
        Logger::Error("IcarianEngine: Profiler Frame not created on thread");
    }

    auto iIter = iter->second->Frames.end();
    while (iIter != iter->second->Frames.begin())
    {
        --iIter;

        if (!iIter->End)
        {
            iIter->EndTime = std::chrono::high_resolution_clock::now();
            iIter->End = true;

            return;
        }
    }

    Logger::Error("IcarianEngine: Profile Start End Frame mismatch");
#endif
}