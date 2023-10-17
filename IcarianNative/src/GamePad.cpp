#include "GamePad.h"

#ifdef WIN32

#else
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>
#endif

GamePad::GamePad()
{
    m_connected = true;
    m_lastButtonState = 0;
    m_buttonState = 0;
    m_index = 0;

    for (uint32_t i = 0; i < GamePadAxis_Last; ++i)
    {
        m_axes[i] = glm::vec2(0.0f);
    }
}
GamePad::~GamePad()
{

}

static constexpr e_GamePadButton GetButtonMap(uint32_t a_button)
{
    switch (a_button)
    {
    case 0:
    {
        return GamePadButton_A;
    }
    case 1:
    {
        return GamePadButton_B;
    }
    case 2:
    {
        return GamePadButton_X;
    }
    case 3:
    {
        return GamePadButton_Y;
    }
    case 4:
    {
        return GamePadButton_LeftBumper;
    }
    case 5:
    {
        return GamePadButton_RightBumper;
    }
    case 6:
    {
        return GamePadButton_Back;
    }
    case 7:
    {
        return GamePadButton_Start;
    }
    case 8:
    {
        return GamePadButton_Guide;
    }
    case 9:
    {
        return GamePadButton_LeftStick;
    }
    case 10:
    {
        return GamePadButton_RightStick;
    }
    }

    return GamePadButton_Last;
}

static constexpr e_GamePadAxis GetAxisVal(uint32_t a_input)
{
    switch (a_input)
    {
    case 0:
    case 1:
    {
        return GamePadAxis_LeftStick;
    }
    case 2:
    {
        return GamePadAxis_LeftTrigger;
    }
    case 3:
    case 4:
    {
        return GamePadAxis_RightStick;
    }
    case 5:
    {
        return GamePadAxis_RightTrigger;
    }
    case 6:
    case 7:
    {
        return GamePadAxis_DPad;
    }
    }

    return GamePadAxis_Last;
}
static constexpr uint32_t GetAxisIndex(uint32_t a_input)
{
    switch (a_input)
    {
    case 0:
    {
        return 1;
    }
    case 1:
    {
        return 0;
    }
    case 3:
    {
        return 1;
    }
    case 4:
    {
        return 0;
    }
    case 6:
    {
        return 0;
    }
    case 7:
    {
        return 1;
    }
    }

    return 0;
}

GamePad* GamePad::GetGamePad(uint32_t a_index)
{
#ifdef WIN32

#else
    const std::string path = "/dev/input/js" + std::to_string(a_index);

    const int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    
    if (fd != -1)
    {
        GamePad* gamePad = new GamePad();

        gamePad->m_fd = fd;
        gamePad->m_index = a_index;
        gamePad->m_name = "Unknown";

        char name[128];
        if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) >= 0)
        {
            gamePad->m_name = name;
        }

        return gamePad;
    }
#endif

    return nullptr;
}

void GamePad::Update()
{
    m_lastButtonState = m_buttonState;

#ifdef WIN32

#else
    struct js_event event;

    while (read(m_fd, &event, sizeof(event)) > 0)
    {
        switch (event.type)
        {
        case JS_EVENT_BUTTON:
        {
            const e_GamePadButton button = GetButtonMap(event.number);

            if (button < GamePadButton_Last)
            {
                if (event.value)
                {
                    m_buttonState |= 0b1 << button;
                }
                else
                {
                    m_buttonState &= ~(0b1 << button);
                }
            }

            break;
        }
        case JS_EVENT_AXIS:
        {
            const e_GamePadAxis axis = GetAxisVal(event.number);
            const uint32_t index = GetAxisIndex(event.number);

            if (axis < GamePadAxis_Last)
            {
                m_axes[axis][index] = event.value / (UINT16_MAX * 0.5f);

                if (axis == GamePadAxis_DPad)
                {
                    if (m_axes[axis][0] < -DPadThreshold)
                    {
                        m_buttonState |= 0b1 << GamePadButton_DPadLeft;
                    }
                    else
                    {
                        m_buttonState &= ~(0b1 << GamePadButton_DPadLeft);
                    }

                    if (m_axes[axis][0] > DPadThreshold)
                    {
                        m_buttonState |= 0b1 << GamePadButton_DPadRight;
                    }
                    else
                    {
                        m_buttonState &= ~(0b1 << GamePadButton_DPadRight);
                    }

                    if (m_axes[axis][1] < -DPadThreshold)
                    {
                        m_buttonState |= 0b1 << GamePadButton_DPadUp;
                    }
                    else
                    {
                        m_buttonState &= ~(0b1 << GamePadButton_DPadUp);
                    }

                    if (m_axes[axis][1] > DPadThreshold)
                    {
                        m_buttonState |= 0b1 << GamePadButton_DPadDown;
                    }
                    else
                    {
                        m_buttonState &= ~(0b1 << GamePadButton_DPadDown);
                    }
                }
            }

            break;
        }
        }
    }

    if (errno == ENODEV)
    {
        m_connected = false;
    }
#endif
}