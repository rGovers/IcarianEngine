// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <string>

#include "EngineInputInteropStructures.h"

class GamePad
{
private:
    static constexpr float DPadThreshold = 0.5f;

#ifndef WIN32
    int         m_fd;
#endif

    uint32_t    m_index;
    std::string m_name;

    glm::vec2   m_axes[GamePadAxis_Last];

    uint32_t    m_lastButtonState;
    uint32_t    m_buttonState;

    bool        m_connected;

    GamePad();

protected:

public:
    ~GamePad();

    inline uint32_t GetIndex() const 
    {
        return m_index;
    }
    inline std::string GetName() const
    {
        return m_name;
    }

    inline bool IsConnected() const
    {
        return m_connected;
    }
    inline bool ButtonStateChanged() const
    {
        return m_lastButtonState != m_buttonState;
    }

    inline bool IsButtonDown(e_GamePadButton a_button) const
    {
        return (m_buttonState & 0b1 << a_button) != 0;
    }
    inline bool IsButtonUp(e_GamePadButton a_button) const
    {
        return (m_buttonState & 0b1 << a_button) == 0;
    }
    inline bool IsButtonPressed(e_GamePadButton a_button) const
    {
        return (m_buttonState & 0b1 << a_button) != 0 && (m_lastButtonState & 0b1 << a_button) == 0;
    }
    inline bool IsButtonReleased(e_GamePadButton a_button) const
    {
        return (m_buttonState & 0b1 << a_button) == 0 && (m_lastButtonState & 0b1 << a_button) != 0;
    }

    static GamePad* GetGamePad(uint32_t a_index);

    inline glm::vec2 GetAxis(e_GamePadAxis a_axis) const
    {
        return m_axes[a_axis];
    }

    void Update();
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