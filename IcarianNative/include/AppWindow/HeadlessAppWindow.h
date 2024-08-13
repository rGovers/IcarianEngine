// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "AppWindow/AppWindow.h"

#include <chrono>
#include <cstdint>
#include <mutex>

#include "Core/IPCPipe.h"
#include "Core/PipeMessage.h"
#include "DataTypes/TArray.h"
#include "Logger.h"
#include "Profiler.h"

class HeadlessAppWindow : public AppWindow
{
private:
    static constexpr int NameMax = 16;
    static constexpr int FrameMax = 64;

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

    static constexpr char PipeName[] = "IcarianEngine-IPC";

    IcarianCore::IPCPipe*                          m_pipe;

    volatile bool                                  m_unlockWindow;    
    bool                                           m_close;

    std::mutex                                     m_fLock;

    TArray<IcarianCore::PipeMessage>               m_queuedMessages;

    char*                                          m_frameData;

    uint32_t                                       m_width;
    uint32_t                                       m_height;

    std::chrono::high_resolution_clock::time_point m_prevTime;
   
    double                                         m_delta;
    double                                         m_time;

    void PushMessageQueue();

    void MessageCallback(const std::string_view& a_message, e_LoggerMessageType a_type);
    void ProfilerCallback(const Profiler::PData& a_profilerData);

    bool PollMessage();

protected:

public:
    HeadlessAppWindow(Application* a_app);
    ~HeadlessAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void SetCursorState(e_CursorState a_state);

    virtual void Update();

    virtual glm::ivec2 GetSize() const;

    virtual bool IsHeadless() const
    {
        return true;
    }

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN
    virtual Array<const char*> GetRequiredVulkanExtenions() const
    {
        return Array<const char*>();
    }
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance)
    {
        return vk::SurfaceKHR();
    }
#endif

    void PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer, double a_delta, double a_time);
};

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