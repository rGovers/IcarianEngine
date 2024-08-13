// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Core/InputBindings.h"

#include "EngineInputInteropStructures.h"

class GamePad;
class RuntimeFunction;

class InputManager
{
public:
    static constexpr uint32_t MaxGamePads = 8;
private:
    RuntimeFunction*            m_mousePressedFunc;
    RuntimeFunction*            m_mouseReleasedFunc;
    RuntimeFunction*            m_keyPressedFunc;
    RuntimeFunction*            m_keyReleasedFunc;

    float                       m_gamePadDeadZone;  

    glm::vec2                   m_curPos;

    unsigned char               m_mouseButton;

    IcarianCore::KeyboardState  m_curKeyState;
    IcarianCore::KeyboardState  m_prevKeyState;

    GamePad*                    m_gamePads[MaxGamePads];

protected:

public:
    InputManager();
    ~InputManager();

    inline void SetCursorPos(const glm::vec2& a_pos)
    {
        m_curPos = a_pos;
    }
    inline glm::vec2 GetCursorPos() const
    {
        return m_curPos;
    }

    void SetMouseButton(e_MouseButton a_button, bool a_state);
    inline bool IsMouseDown(e_MouseButton a_button) const
    {
        return m_mouseButton & 0b1 << (a_button * 2);
    }
    inline bool IsMousePressed(e_MouseButton a_button) const
    {
        return m_mouseButton & 0b1 << (a_button * 2 + 0) && !(m_mouseButton & 0b1 << (a_button * 2 + 1));
    }
    inline bool IsMouseReleased(e_MouseButton a_button) const
    {
        return !(m_mouseButton & 0b1 << (a_button * 2 + 0)) && m_mouseButton & 0b1 << (a_button * 2 + 1);
    }

    void SetKeyboardKey(e_KeyCode a_keyCode, bool a_state);
    inline bool IsKeyDown(e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyPressed(e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode) && !m_prevKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyReleased(e_KeyCode a_keyCode) const
    {
        return !m_curKeyState.IsKeyDown(a_keyCode) && m_prevKeyState.IsKeyDown(a_keyCode);
    }

    bool IsGamePadConnected(e_GamePadSlot a_slot) const;

    glm::vec2 GetGamePadAxis(e_GamePadSlot a_slot, e_GamePadAxis a_axis) const;

    bool IsGamePadButtonDown(e_GamePadSlot a_slot, e_GamePadButton a_button) const;
    bool IsGamePadButtonPressed(e_GamePadSlot a_slot, e_GamePadButton a_button) const;
    bool IsGamePadButtonReleased(e_GamePadSlot a_slot, e_GamePadButton a_button) const;

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