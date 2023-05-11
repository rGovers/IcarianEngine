#pragma once

#include <cstdint>

#include "AppWindow/AppWindow.h"

class Config;
class InputManager;
class ObjectManager;
class RenderEngine;
class RuntimeManager;

class Application
{
private:
    AppWindow*      m_appWindow;

    bool            m_close;

    Config*         m_config;
    InputManager*   m_inputManager;
    ObjectManager*  m_objectManager;
    RuntimeManager* m_runtime;
    RenderEngine*   m_renderEngine;

protected:

public:
    Application(Config* a_config);
    ~Application();

    void Run(int32_t a_argc, char* a_argv[]);

    inline uint32_t GetWidth() const
    {
        return (uint32_t)m_appWindow->GetSize().x;
    }
    inline uint32_t GetHeight() const
    {
        return (uint32_t)m_appWindow->GetSize().y;
    }

    inline void Resize(uint32_t a_width, uint32_t a_height)
    {
        m_appWindow->Resize(a_width, a_height);
    }
    inline void SetFullscreen(const AppMonitor& a_monitor, bool a_state, uint32_t a_width, uint32_t a_height)
    {
        m_appWindow->SetFullscreen(a_monitor, a_state, a_width, a_height);
    }

    inline bool IsHeadless() const
    {
        return m_appWindow->IsHeadless();
    }

    inline AppMonitor* GetMonitors(int* a_count) const
    {
        return m_appWindow->GetMonitors(a_count);
    }

    inline void Close()
    {
        m_close = true;
    }

    inline InputManager* GetInputManager() const
    {
        return m_inputManager;
    }
    inline RuntimeManager* GetRuntime() const
    {
        return m_runtime;
    }
};