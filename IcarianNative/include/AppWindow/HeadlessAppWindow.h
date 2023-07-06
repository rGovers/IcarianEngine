#pragma once

#include "AppWindow/AppWindow.h"

#include <chrono>
#include <cstdint>
#include <mutex>

#include "DataTypes/TArray.h"
#include "Flare/IPCPipe.h"
#include "Flare/PipeMessage.h"
#include "Flare/WindowsHeaders.h"
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

    static constexpr std::string_view PipeName = "IcarianEngine-IPC";

    FlareBase::IPCPipe*                            m_pipe;

    volatile bool                                  m_unlockWindow;    
    bool                                           m_close;

    std::mutex                                     m_fLock;

    TArray<FlareBase::PipeMessage>                 m_queuedMessages;

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

    virtual void Update();

    virtual glm::ivec2 GetSize() const;

    virtual bool IsHeadless() const
    {
        return true;
    }
    virtual std::vector<const char*> GetRequiredVulkanExtenions() const
    {
        return std::vector<const char*>();
    }
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance)
    {
        return vk::SurfaceKHR();
    }

    void PushFrameData(uint32_t a_width, uint32_t a_height, const char* a_buffer, double a_delta, double a_time);
};