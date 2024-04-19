#pragma once

#include <cstdint>

#include "EngineInputInteropStructures.h"

namespace IcarianCore
{    
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
