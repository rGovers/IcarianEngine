#pragma once

#include <chrono>
#include <functional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if !defined(NDEBUG) && !defined(FLARENATIVE_ENABLE_PROFILER)
#define FLARENATIVE_ENABLE_PROFILER
#endif

class RuntimeManager;

struct ProfileFrame
{
    bool End;
    uint32_t Stack;
    std::string Name;
    std::chrono::high_resolution_clock::time_point StartTime;
    std::chrono::high_resolution_clock::time_point EndTime;
};

class Profiler
{
public:
    struct PData
    {
        std::string Name;
        std::vector<ProfileFrame> Frames;
    };

    typedef std::function<void(const PData&)> Callback;

private:
    static Profiler* Instance;

    std::shared_mutex                           m_mutex;
    std::unordered_map<std::thread::id, PData*> m_data;

    Profiler(RuntimeManager* a_runtimeManager);

protected:

public:
    ~Profiler();

    static Callback* CallbackFunc;

    static void Init(RuntimeManager* a_runtimeManager);
    static void Destroy();

    static void Start(const std::string_view& a_name);
    static void Stop();

    static void StartFrame(const std::string_view& a_name);
    static void StopFrame();
};

struct StackProfilerFrame 
{ 
    StackProfilerFrame(const std::string_view& a_name)
    { 
        Profiler::StartFrame(a_name); 
    } 
    ~StackProfilerFrame() 
    { 
        Profiler::StopFrame(); 
    } 
};

#ifndef PROFILESTACK
#ifdef FLARENATIVE_ENABLE_PROFILER
#define PROFILESTACK(str) volatile StackProfilerFrame stackPFrame = StackProfilerFrame(str)
#else
#define PROFILESTACK(str) void(0)
#endif
#endif