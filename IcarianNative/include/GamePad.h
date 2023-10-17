#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>
#include <string>

#ifdef WIN32

#else

#endif

#include "EngineInputInteropStructures.h"

class GamePad
{
private:
    static constexpr float DPadThreshold = 0.5f;

#ifdef WIN32

#else
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