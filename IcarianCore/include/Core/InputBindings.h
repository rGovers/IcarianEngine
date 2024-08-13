// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

#include "Core/Bitfield.h"

#include "EngineInputInteropStructures.h"

namespace IcarianCore
{    
    // Only need up, down, press and release so only 2 needed to track keyboard state so ~30 bytes
    // Class exists so I dont have to shift about to find state info
    class KeyboardState
    {
    public:
        static constexpr uint32_t ElementCount = (KeyCode_Last / 8) + 1;
    
    private:
        uint8_t m_state[ElementCount];
    
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
    
        static KeyboardState FromData(const uint8_t* a_data);
    
        inline const uint8_t* ToData() const
        {
            return m_state;
        }
    
        inline void SetKey(e_KeyCode a_keyCode, bool a_state)
        {
            const unsigned int index = a_keyCode / 8;
            const unsigned int offset = a_keyCode % 8;
    
            ITOGGLEBIT(a_state, m_state[index], offset);
        }
        inline bool IsKeyDown(e_KeyCode a_keyCode) const
        {
            const unsigned int index = a_keyCode / 8;
            const unsigned int offset = a_keyCode % 8;
    
            return IISBITSET(m_state[index], offset);
        }
    };
}

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