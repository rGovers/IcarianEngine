// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

#include "AppWindow/AppWindow.h"

class AudioEngine;
class Config;
class InputManager;
class Navigation;
class NetworkManager;
class ObjectManager;
class PhysicsEngine;
class RenderEngine;
class RuntimeManager;

class Application
{
private:
    AppWindow*      m_appWindow;

    bool            m_close;
    e_CursorState   m_cursorState;

    Config*         m_config;
    InputManager*   m_inputManager;

    Navigation*     m_navigation;
    AudioEngine*    m_audioEngine;
    PhysicsEngine*  m_physicsEngine;
    RenderEngine*   m_renderEngine;
    NetworkManager* m_networkManager;

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

    inline e_CursorState GetCursorState() const
    {
        return m_cursorState;
    }
    void SetCursorState(e_CursorState a_state);

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