#pragma once

#include "Rendering/UI/UIElement.h"

#include <string>

class TextUIElement : public UIElement
{
private:
    RenderEngine*  m_lastRenderEngine;

    std::u32string m_text;
    
    uint32_t       m_textureAddr;
    uint32_t       m_samplerAddr;

    float          m_fontSize;
    uint32_t       m_fontAddr;

    bool           m_refresh;

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

    inline std::u32string GetText() const
    {
        return m_text;
    }
    void SetText(const std::u32string_view& a_text);

    virtual void Update(RenderEngine* a_renderEngine);
};