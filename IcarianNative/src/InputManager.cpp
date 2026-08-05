// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "InputManager.h"

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "GamePad.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef WIN32
#include "LibXInput.h"
#endif

#include "EngineInputInterop.h"
#include "EngineInputInteropStructures.h"

#define INPUTMANAGER_BINDING_FUNCTION_TABLE(F) \
    F(glm::vec2, IcarianEngine, Input, GetCursorPos, { return Instance->GetCursorPos(); }) \
    \
    F(uint32_t, IcarianEngine, Input, GetMouseDownState, { return (uint32_t)Instance->IsMouseDown((e_MouseButton)a_button); }, uint32_t a_button) \
    F(uint32_t, IcarianEngine, Input, GetMousePressedState, { return (uint32_t)Instance->IsMousePressed((e_MouseButton)a_button); }, uint32_t a_button) \
    F(uint32_t, IcarianEngine, Input, GetMouseReleasedState, { return (uint32_t)Instance->IsMouseReleased((e_MouseButton)a_button); }, uint32_t a_button) \
    \
    F(uint32_t, IcarianEngine, Input, GetKeyDownState, { return (uint32_t)Instance->IsKeyDown((e_KeyCode)a_keyCode); }, uint32_t a_keyCode) \
    F(uint32_t, IcarianEngine, Input, GetKeyPressedState, { return (uint32_t)Instance->IsKeyPressed((e_KeyCode)a_keyCode); }, uint32_t a_keyCode) \
    F(uint32_t, IcarianEngine, Input, GetKeyReleasedState, { return (uint32_t)Instance->IsKeyReleased((e_KeyCode)a_keyCode); }, uint32_t a_keyCode) \

static InputManager* Instance = nullptr;

ENGINE_INPUT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
INPUTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

InputManager::InputManager()
{
    Instance = this;

    TRACE("Initializing Input Manager");

#ifdef WIN32
    LibXInput::Init();
#endif

    m_mouseButton = 0;
    m_curPos = glm::vec2(0.0f);

    m_gamePadDeadZone = 0.1f;

    ENGINE_INPUT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    INPUTMANAGER_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    m_mousePressedFunc = RuntimeManager::GetFunction("IcarianEngine", "Input", ":MousePressedEvent(uint)");
    m_mouseReleasedFunc = RuntimeManager::GetFunction("IcarianEngine", "Input", ":MouseReleasedEvent(uint)");
    m_keyPressedFunc = RuntimeManager::GetFunction("IcarianEngine", "Input", ":KeyPressedEvent(uint)");
    m_keyReleasedFunc = RuntimeManager::GetFunction("IcarianEngine", "Input", ":KeyReleasedEvent(uint)");

    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        m_gamePads[i] = GamePad::GetGamePad(i);
    }
}
InputManager::~InputManager()
{
    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (m_gamePads[i] != nullptr)
        {
            delete m_gamePads[i];
            m_gamePads[i] = nullptr;
        }
    }

#ifdef WIN32
    LibXInput::Destroy();
#endif

    IcarianCore::MallocAllocator::Instance->Destroy(m_mousePressedFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_mouseReleasedFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_keyPressedFunc);
    IcarianCore::MallocAllocator::Instance->Destroy(m_keyReleasedFunc);
}

void InputManager::SetMouseButton(e_MouseButton a_button, bool a_state)
{
    const uint32_t buttonIndex = a_button * 2 + 0;
    const uint32_t prevButtonIndex = a_button * 2 + 1;

    const bool prevState = m_mouseButton & 0b1 << buttonIndex;
    if (a_state)
    {
        if (!(prevState))
        {
            uint32_t but = (uint32_t)a_button;
            void* args[] =
            {
                &but
            };

            m_mousePressedFunc->Exec(args);
        }

        m_mouseButton |= 0b1 << buttonIndex;
    }
    else
    {
        if (prevState)
        {
            uint32_t but = (uint32_t)a_button;
            void* args[] =
            {
                &but
            };

            m_mouseReleasedFunc->Exec(args);
        }

        m_mouseButton &= ~(0b1 << buttonIndex);
    }

    if (prevState)
    {
        m_mouseButton |= 0b1 << prevButtonIndex;
    }
    else
    {
        m_mouseButton &= ~(0b1 << prevButtonIndex);
    }
}

void InputManager::SetKeyboardKey(e_KeyCode a_keyCode, bool a_state)
{
    const bool prevState = m_curKeyState.IsKeyDown(a_keyCode);

    m_curKeyState.SetKey(a_keyCode, a_state);
    if (a_state)
    {
        if (!prevState)
        {
            uint32_t key = (uint32_t)a_keyCode;
            void* args[]
            {
                &key
            };

            m_keyPressedFunc->Exec(args);
        }
    }
    else if (prevState)
    {
        uint32_t key = (uint32_t)a_keyCode;
        void* args[]
        {
            &key
        };

        m_keyReleasedFunc->Exec(args);
    }

    m_prevKeyState.SetKey(a_keyCode, prevState);
}

bool InputManager::IsGamePadConnected(e_GamePadSlot a_slot) const
{
    e_GamePadSlot mask = (e_GamePadSlot)0;

    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (m_gamePads[i] != nullptr && m_gamePads[i]->IsConnected())
        {
            mask = (e_GamePadSlot)(mask | 0b1 << i);
        }
    }

    return (a_slot & mask) == a_slot;
}

glm::vec2 InputManager::GetGamePadAxis(e_GamePadSlot a_slot, e_GamePadAxis a_axis) const
{
    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (a_slot & 0b1 << i && m_gamePads[i] != nullptr)
        {
            const glm::vec2 axis = m_gamePads[i]->GetAxis(a_axis);

            if (glm::length(axis) > m_gamePadDeadZone)
            {
                return axis;
            }
        }
    }

    return glm::vec2(0.0f);
}

bool InputManager::IsGamePadButtonDown(e_GamePadSlot a_slot, e_GamePadButton a_button) const
{
    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (a_slot & 0b1 << i && m_gamePads[i] != nullptr)
        {
            if (m_gamePads[i]->IsButtonDown(a_button))
            {
                return true;
            }
        }
    }

    return false;
}
bool InputManager::IsGamePadButtonPressed(e_GamePadSlot a_slot, e_GamePadButton a_button) const
{
    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (a_slot & 0b1 << i && m_gamePads[i] != nullptr)
        {
            if (m_gamePads[i]->IsButtonPressed(a_button))
            {
                return true;
            }
        }
    }

    return false;
}
bool InputManager::IsGamePadButtonReleased(e_GamePadSlot a_slot, e_GamePadButton a_button) const
{
    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (a_slot & 0b1 << i && m_gamePads[i] != nullptr)
        {
            if (m_gamePads[i]->IsButtonReleased(a_button))
            {
                return true;
            }
        }
    }

    return false;
}

void InputManager::Update()
{
    // Required for frame events can get multiple press and release events cause majority of the time the engine runs faster then the editor
    for (uint32_t i = 0; i < MouseButton_Last; ++i)
    {
        if (m_mouseButton & 0b1 << (i * 2))
        {
            m_mouseButton |= 0b1 << (i * 2 + 1);
        }
        else
        {
            m_mouseButton &= ~(0b1 << (i * 2 + 1));
        }
    }

    m_prevKeyState = m_curKeyState;

    for (uint32_t i = 0; i < MaxGamePads; ++i)
    {
        if (m_gamePads[i] != nullptr)
        {
            m_gamePads[i]->Update();

            if (!m_gamePads[i]->IsConnected())
            {
                delete m_gamePads[i];
                m_gamePads[i] = nullptr;
            }
        }
        else
        {
            m_gamePads[i] = GamePad::GetGamePad(i);
        }
    }
}

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
