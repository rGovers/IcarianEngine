// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <unordered_map>

#include "Core/IcarianDefer.h"
#include "Core/MemoryUsageFrame.h"
#include "DataTypes/Array.h"
#include "DataTypes/COWString.h"
#include "DataTypes/SpinLock.h"

#if !defined(NDEBUG) && !defined(ICARIANNATIVE_ENABLE_PROFILER)
#define ICARIANNATIVE_ENABLE_PROFILER
#endif

struct ProfileFrame
{
    COWU8String Name;
    double Duration;
    std::chrono::high_resolution_clock::time_point StartTime;
    uint32_t Stack;
    bool End;
};

enum e_ProfilerMemoryFrame
{
    ProfilerMemoryFrame_CSharp,
    ProfilerMemoryFrame_Audio,
    ProfilerMemoryFrame_Rendering,
    ProfilerMemoryFrame_Physics,
};

class Profiler
{
public:
    struct PData
    {
        COWU8String Name;
        Array<ProfileFrame> Frames;
    };

    typedef std::function<void(const PData&)> Callback;

private:
    SharedSpinLock                             m_lock;

    std::unordered_map<std::thread::id, PData> m_data;
    IcarianCore::MemoryUsageFrame              m_memoryFrame;

protected:

public:
    Profiler();
    ~Profiler();

    static Callback* CallbackFunc;

    static void Init();
    static void Destroy();

    static void Start(const char* a_name);
    static void Start(const COWU8String& a_name);
    static void Stop();

    static void PushMemoryFrame(e_ProfilerMemoryFrame a_frame, uint64_t a_size);
    static IcarianCore::MemoryUsageFrame GetMemoryFrames();

    static void StartFrame(const char* a_name);
    static void StartFrame(const COWU8String& a_name);
    static void StopFrame();
};

#ifndef PROFILESTACK
#ifdef ICARIANNATIVE_ENABLE_PROFILER
#define ICARIAN_PROFILE_VAL_NAMEI(i) pFrame##i
#define ICARIAN_PROFILE_VAL_NAME(i) ICARIAN_PROFILE_VAL_NAMEI(i)

#define PROFILESTACK(str) Profiler::StartFrame(str); IDEFER(Profiler::StopFrame())
#else
#define PROFILESTACK(str) void(0)
#endif
#endif

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