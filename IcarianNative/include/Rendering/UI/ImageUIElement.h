#pragma once

#include "Rendering/UI/UIElement.h"

class ImageUIElement : public UIElement
{
private:
    uint32_t m_samplerAddr;

protected:

public:
    ImageUIElement();
    virtual ~ImageUIElement();

    virtual e_UIElementType GetType() const
    {
        return UIElementType_Image;
    }

    inline uint32_t GetSamplerAddr() const
    {
        return m_samplerAddr;
    }
    inline void SetSamplerAddr(uint32_t a_addr)
    {
        m_samplerAddr = a_addr;
    }

    virtual void Update(RenderEngine* a_renderEngine);
};