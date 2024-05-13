#include "Core/InputBindings.h"

namespace IcarianCore
{
    KeyboardState KeyboardState::FromData(const uint8_t* a_data)
    {
        KeyboardState state;
            
        for (uint32_t i = 0; i < ElementCount; ++i)
        {
            state.m_state[i] = a_data[i];
        }

        return state;
    }
}