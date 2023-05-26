#pragma once

#include "Rendering/UI/UIElement.h"

#include <string>

class TextUIElement : public UIElement
{
private:
    std::u32string m_text;
    uint32_t       m_textureAddr;

    bool           m_refresh;

protected:

public:
    TextUIElement();
    virtual ~TextUIElement();

    virtual e_UIElementType GetType() const
    {
        return UIElementType_Text;
    }

    virtual void Update(RenderEngine* a_renderEngine);
};