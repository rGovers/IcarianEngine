// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "AppWindow/AppWindow.h"

#include <chrono>
#include <cstdint>
#include <mutex>

#include "Core/Bitfield.h"
#include "Core/DMASwapBuffer.h"
#include "Core/CommunicationPipe.h"
#include "Core/PipeMessage.h"
#include "DataTypes/TArray.h"
#include "Logger.h"
#include "Profiler.h"

class Config;
class RingAllocator;
class RuntimeFunction;

class HeadlessAppWindow : public AppWindow
{
private:
    static constexpr uint16_t NameMax = 16;
    static constexpr uint16_t FrameMax = 64;

    struct ProfileTFrame
    {
        char Name[NameMax];
        float Time;
        uint8_t Stack;
    };

    struct ProfileScope
    {
        char Name[NameMax];
        uint16_t FrameCount;
        ProfileTFrame Frames[FrameMax];
    };

    static constexpr char PipeName[] = "IcarianEditor-IPC";
    static constexpr uint32_t CloseBit = 0;
    static constexpr uint32_t RemoteBit = 1;

    IcarianCore::CommunicationPipe*                m_pipe;

    TArray<IcarianCore::PipeMessage>               m_queuedMessages;

    RuntimeFunction*                               m_runtimeMessageReceive;

#ifndef ICARIANNATIVE_ENABLE_DMA
    std::mutex                                     m_fLock;
    volatile bool                                  m_unlockWindow;
    uint64_t                                       m_windowFrame;
    uint64_t                                       m_gpuFrame;
    char*                                          m_frameData;
#endif

    RingAllocator*                                 m_msgAllocator;

    uint32_t                                       m_width;
    uint32_t                                       m_height;

    std::chrono::high_resolution_clock::time_point m_prevTime;
   
    double                                         m_delta;
    double                                         m_time;

    uint8_t                                        m_flags;
    SpinLock                                       m_msgAllocatorLock;

    void PushMessageQueue();

    void MessageCallback(const COWU8String& a_message, IcarianCore::e_LoggerMessageType a_type, uint32_t a_stackTraceCount, const char* const* a_stackTrace);
    void ProfilerCallback(const Profiler::PData& a_profilerData);

    bool PollMessage();

protected:

public:
    HeadlessAppWindow(Application* a_app, Config* a_config);
    ~HeadlessAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void SetCursorState(e_CursorState a_state);

    virtual void Update();

    virtual uint32_t GetWidth() const
    {
        return m_width;
    }
    virtual uint32_t GetHeight() const
    {
        return m_height;
    }

    virtual bool IsHeadless() const
    {
        return true;
    }

    inline bool IsRemote() const
    {
        return IISBITSET(m_flags, RemoteBit);
    }

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    virtual Array<const char*> GetRequiredVulkanExtenions() const;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance)
    {
        return vk::SurfaceKHR();
    }
#endif

    void PushFrameInfo(double a_delta, double a_time);

#ifdef ICARIANNATIVE_ENABLE_DMA
    void PushSwapBufferFD(const DMASwapBufferFD& a_swapbuffer);
    void FlushSwapBufferFD();

#ifdef WIN32
    void PushSwapBufferHandle(const DMASwapBufferHandle& a_swapBuffer);
#endif
    void FlushSwapBufferHandle();

    void DMASwap();
#else
    void PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer);
#endif
};

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