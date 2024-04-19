#include "Core/InputBindings.h"

namespace IcarianCore
{
    KeyboardState KeyboardState::FromData(const unsigned char* a_data)
    {
        KeyboardState state;
            
        for (unsigned int i = 0; i < ElementCount; ++i)
        {
            state.m_state[i] = a_data[i];
        }

        return state;
    }
}