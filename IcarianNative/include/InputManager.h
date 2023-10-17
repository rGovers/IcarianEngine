#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Flare/InputBindings.h"

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

    FlareBase::KeyboardState    m_curKeyState;
    FlareBase::KeyboardState    m_prevKeyState;

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

    void SetMouseButton(FlareBase::e_MouseButton a_button, bool a_state);
    inline bool IsMouseDown(FlareBase::e_MouseButton a_button) const
    {
        return m_mouseButton & 0b1 << (a_button * 2);
    }
    inline bool IsMousePressed(FlareBase::e_MouseButton a_button) const
    {
        return m_mouseButton & 0b1 << (a_button * 2 + 0) && !(m_mouseButton & 0b1 << (a_button * 2 + 1));
    }
    inline bool IsMouseReleased(FlareBase::e_MouseButton a_button) const
    {
        return !(m_mouseButton & 0b1 << (a_button * 2 + 0)) && m_mouseButton & 0b1 << (a_button * 2 + 1);
    }

    void SetKeyboardKey(FlareBase::e_KeyCode a_keyCode, bool a_state);
    inline bool IsKeyDown(FlareBase::e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyPressed(FlareBase::e_KeyCode a_keyCode) const
    {
        return m_curKeyState.IsKeyDown(a_keyCode) && !m_prevKeyState.IsKeyDown(a_keyCode);
    }
    inline bool IsKeyReleased(FlareBase::e_KeyCode a_keyCode) const
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