#pragma once

#include <cstdint>

namespace FlareBase
{
    enum e_MouseButton : uint16_t
    {
        MouseButton_Left = 0,
        MouseButton_Middle = 1,
        MouseButton_Right = 2,
        MouseButton_Last
    };
    
    enum e_CursorState : uint16_t
    {
        CursorState_Normal = 0,
        CursorState_Hidden = 1,
        CursorState_Locked = 2
    };

    enum e_KeyCode : uint16_t
    {
        KeyCode_Null = UINT16_MAX,
    
        KeyCode_Space = 0, 
        KeyCode_Apostrophe = 1, /* ' */
        KeyCode_Comma = 2, /* , */
        KeyCode_Minus = 3, /* - */
        KeyCode_Equals = 4, /* = */
        KeyCode_Period = 5, /* . */
        KeyCode_ForwardSlash = 6, /* / */ 
        KeyCode_BackSlash = 7, /* \ */
        KeyCode_LeftBracket = 8, /* [ */
        KeyCode_RightBracket = 9, /* ] */
        KeyCode_Semicolon = 10, /* ; */
        KeyCode_Accent = 11, /* ` */
    
        KeyCode_A = 12,
        KeyCode_B = 13,
        KeyCode_C = 14,
        KeyCode_D = 15,
        KeyCode_E = 16,
        KeyCode_F = 17,
        KeyCode_G = 18,
        KeyCode_H = 19,
        KeyCode_I = 20,
        KeyCode_J = 21,
        KeyCode_K = 22,
        KeyCode_L = 23,
        KeyCode_M = 24,
        KeyCode_N = 25,
        KeyCode_O = 26,
        KeyCode_P = 27,
        KeyCode_Q = 28,
        KeyCode_R = 29,
        KeyCode_S = 30,
        KeyCode_T = 31,
        KeyCode_U = 32,
        KeyCode_V = 33,
        KeyCode_W = 34,
        KeyCode_X = 35,
        KeyCode_Y = 36,
        KeyCode_Z = 37,
    
        KeyCode_0 = 38,
        KeyCode_1 = 39,
        KeyCode_2 = 40,
        KeyCode_3 = 41,
        KeyCode_4 = 42,
        KeyCode_5 = 43,
        KeyCode_6 = 44,
        KeyCode_7 = 45,
        KeyCode_8 = 46,
        KeyCode_9 = 47,
    
        KeyCode_PrintableEnd = KeyCode_9 + 1,
    
        KeyCode_KeypadDecimal = 48, /* . */
        KeyCode_KeypadDivide = 49, /* / */
        KeyCode_KeypadMultiply = 50, /* * */
        KeyCode_KeypadSubtract = 51, /* - */
        KeyCode_KeypadAdd = 52, /* + */
        KeyCode_KeypadEquals = 53, /* = */
        KeyCode_KeypadEnter = 54,
    
        KeyCode_Keypad0 = 55,
        KeyCode_Keypad1 = 56,
        KeyCode_Keypad2 = 57,
        KeyCode_Keypad3 = 58,
        KeyCode_Keypad4 = 59,
        KeyCode_Keypad5 = 60,
        KeyCode_Keypad6 = 61,
        KeyCode_Keypad7 = 62,
        KeyCode_Keypad8 = 63,
        KeyCode_Keypad9 = 64,
    
        KeyCode_Escape = 65,
        KeyCode_Enter = 66,
        KeyCode_Tab = 67,
        KeyCode_Backspace = 68,
        KeyCode_Insert = 69,
        KeyCode_Delete = 70,
        KeyCode_Home = 71,
        KeyCode_End = 72,
        KeyCode_PageUp = 73,
        KeyCode_PageDown = 74,
        KeyCode_LeftArrow = 75,
        KeyCode_RightArrow = 76,
        KeyCode_UpArrow = 77,
        KeyCode_DownArrow = 78,
        KeyCode_CapsLock = 79,
        KeyCode_NumLock = 80,
        KeyCode_ScrollLock = 81,
        KeyCode_PrintScreen = 82,
        KeyCode_PauseBreak = 83,
    
        KeyCode_LeftShift = 84,
        KeyCode_LeftCtrl = 85,
        KeyCode_LeftAlt = 86,
        KeyCode_LeftSuper = 87, /* Windows Key */
    
        KeyCode_RightShift = 88,
        KeyCode_RightCtrl = 89,
        KeyCode_RightAlt = 90,
        KeyCode_RightSuper = 91, /* Windows Key */
    
        KeyCode_F1 = 92,
        KeyCode_F2 = 93,
        KeyCode_F3 = 94,
        KeyCode_F4 = 95,
        KeyCode_F5 = 96,
        KeyCode_F6 = 97,
        KeyCode_F7 = 98,
        KeyCode_F8 = 99,
        KeyCode_F9 = 100,
        KeyCode_F10 = 101,
        KeyCode_F11 = 102,
        KeyCode_F12 = 103,
        KeyCode_F13 = 104,
        KeyCode_F14 = 105,
        KeyCode_F15 = 106,
        KeyCode_F16 = 107,
        KeyCode_F17 = 108,
        KeyCode_F18 = 109,
        KeyCode_F19 = 110,
        KeyCode_F20 = 111,
        KeyCode_F21 = 112,
        KeyCode_F22 = 113,
        KeyCode_F23 = 114,
        KeyCode_F24 = 115,
        KeyCode_F25 = 116,
    
        KeyCode_Menu = 117,
    
        KeyCode_Last
    };
    
    // Only need up, down, press and release so only 2 needed to track keyboard state so ~30 bytes
    // Class exists so I dont have to shift about to find state info
    class KeyboardState
    {
    public:
        static constexpr unsigned int ElementCount = (KeyCode_Last / 8) + 1;
    
    private:
        unsigned char m_state[ElementCount];
    
    protected:
    
    public:
        KeyboardState()
        {
            for (unsigned int i = 0; i < ElementCount; ++i)
            {
                m_state[i] = 0;
            }
        }
        KeyboardState(const KeyboardState& a_other)
        {
            for (unsigned int i = 0; i < ElementCount; ++i)
            {
                m_state[i] = a_other.m_state[i];
            }
        }
        ~KeyboardState() { }
    
        static KeyboardState FromData(const unsigned char* a_data);
    
        inline unsigned char* ToData()
        {
            return m_state;
        }
    
        inline void SetKey(e_KeyCode a_keyCode, bool a_state)
        {
            const unsigned int index = a_keyCode / 8;
            const unsigned int offset = a_keyCode % 8;
    
            if (a_state)
            {
                m_state[index] |= 0b1 << offset;
            }
            else
            {
                m_state[index] &= ~(0b1 << offset);
            }
        }
        inline bool IsKeyDown(e_KeyCode a_keyCode) const
        {
            const unsigned int index = a_keyCode / 8;
            const unsigned int offset = a_keyCode % 8;
    
            return m_state[index] & 0b1 << offset;
        }
    };
}
