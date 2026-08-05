// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <chrono>

#include "Core/DataTypes/Array.h"
#include "Core/DataTypes/COWString.h"
#include "Core/DataTypes/Dictionary.h"
#include "Core/DataTypes/SpinLock.h"
#include "Core/IcarianDefer.h"
#include "Core/MemoryUsageFrame.h"

#if !defined(NDEBUG) && !defined(ICARIANNATIVE_ENABLE_PROFILER)
#define ICARIANNATIVE_ENABLE_PROFILER
#endif

struct ProfileFrame
{
    IcarianCore::COWU8String Name;
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
    ProfilerMemoryFrame_FileCache,
};

struct ProfilerGPUFrameItem
{
    IcarianCore::COWU8String Name;
    double Duration;
};

struct ProfilerGPUFrameData
{
    IcarianCore::COWU8String Name;
    IcarianCore::Array<ProfilerGPUFrameItem> Items;
};

struct ProfilerCPUData
{
    IcarianCore::COWU8String Name;
    IcarianCore::Array<ProfileFrame> Frames;
};

class Profiler
{
public:
    class CPUCallbackItem
    {
    private:

    protected:

    public:
        virtual void Execute(const ProfilerCPUData& a_data) = 0;
    };

    class GPUCallbackItem
    {
    private:

    protected:

    public:
        virtual void Execute(const ProfilerGPUFrameData* a_data, uint32_t a_count) = 0;
    };

    class GPUETECallbackItem
    {
    private:

    protected:

    public:
        virtual void Execute(float a_time) = 0;
    };

private:
    IcarianCore::SharedSpinLock                               m_lock;

    IcarianCore::Dictionary<std::thread::id, ProfilerCPUData> m_data;
    IcarianCore::MemoryUsageFrame                             m_memoryFrame;

protected:

public:
    Profiler();
    ~Profiler();

    static CPUCallbackItem* CPUCallback;
    static GPUCallbackItem* GPUCallback;
    static GPUETECallbackItem* GPUETECallback;

    static void Init();
    static void Destroy();

    static void Start(const char* a_name);
    static void Start(const IcarianCore::COWU8String& a_name);
    static void Stop();

    static void PushMemoryFrame(e_ProfilerMemoryFrame a_frame, uint64_t a_size);
    static IcarianCore::MemoryUsageFrame GetMemoryFrames();

    static void StartFrame(const char* a_name);
    static void StartFrame(const IcarianCore::COWU8String& a_name);
    static void StopFrame();

    static void PushGPUData(const ProfilerGPUFrameData* a_data, uint32_t a_count);
    static void PushGPUEndToEndTime(float a_time);
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
