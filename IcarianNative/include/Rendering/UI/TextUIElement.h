// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "Rendering/UI/UIElement.h"

#include <string>

class TextUIElement : public UIElement
{
private:
    static constexpr uint32_t RefreshBit = 0;
    static constexpr uint32_t ValidBit = 1;

    RenderEngine*  m_lastRenderEngine;

    SharedSpinLock m_lock;

    std::u32string m_text;
    
    uint32_t       m_textureAddr;
    uint32_t       m_samplerAddr;

    float          m_fontSize;
    uint32_t       m_fontAddr;

    uint8_t        m_flags;

protected:

public:
    TextUIElement();
    virtual ~TextUIElement();

    virtual e_UIElementType GetType() const
    {
        return UIElementType_Text;
    }

    inline uint32_t GetSamplerAddr() const
    {
        return m_samplerAddr;
    }

    inline uint32_t GetFontAddr() const
    {
        return m_fontAddr;
    }
    void SetFontAddr(uint32_t a_addr);

    inline float GetFontSize() const
    {
        return m_fontSize;
    }
    inline void SetFontSize(float a_size)
    {
        m_fontSize = a_size;
    }

    inline bool IsValid() const
    {
        return m_flags & 0b1 << ValidBit;
    }

    std::u32string GetText();
    void SetText(const std::u32string_view& a_text);

    virtual void Update(RenderEngine* a_renderEngine);
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