// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <chrono>
#include <functional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if !defined(NDEBUG) && !defined(ICARIANNATIVE_ENABLE_PROFILER)
#define ICARIANNATIVE_ENABLE_PROFILER
#endif

struct ProfileFrame
{
    std::string Name;
    double Duration;
    std::chrono::high_resolution_clock::time_point StartTime;
    uint32_t Stack;
    bool End;
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

    std::shared_mutex                          m_mutex;
    std::unordered_map<std::thread::id, PData> m_data;

    Profiler();

protected:

public:
    ~Profiler();

    static Callback* CallbackFunc;

    static void Init();
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
#ifdef ICARIANNATIVE_ENABLE_PROFILER
#define ICARIAN_PROFILE_VAL_NAMEI(i) pFrame##i
#define ICARIAN_PROFILE_VAL_NAME(i) ICARIAN_PROFILE_VAL_NAMEI(i)

#define PROFILESTACK(str) volatile StackProfilerFrame ICARIAN_PROFILE_VAL_NAME(__COUNTER__) = StackProfilerFrame(str)
#else
#define PROFILESTACK(str) void(0)
#endif
#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
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