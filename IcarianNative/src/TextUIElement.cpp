#include "Rendering/UI/TextUIElement.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/Font.h"

TextUIElement::TextUIElement() : UIElement()
{
    m_fontAddr = -1;
    m_fontSize = 10.0f;

    m_textureAddr = -1;
    m_samplerAddr = -1;

    m_refresh = false;

    m_lastRenderEngine = nullptr;
}
TextUIElement::~TextUIElement()
{
    if (m_textureAddr != -1)
    {
        ICARIAN_ASSERT_MSG(m_lastRenderEngine != nullptr, "TextUIElement last render engine is null");

        m_lastRenderEngine->DestroyTexture(m_textureAddr);
    }
    if (m_samplerAddr != -1)
    {
        ICARIAN_ASSERT_MSG(m_lastRenderEngine != nullptr, "TextUIElement last render engine is null");

        m_lastRenderEngine->DestroyTextureSampler(m_samplerAddr);
    }
}

void TextUIElement::SetFontAddr(uint32_t a_addr)
{
    m_fontAddr = a_addr;

    m_refresh = true;
}
void TextUIElement::SetText(const std::u32string_view& a_text)
{
    m_text = std::u32string(a_text);

    m_refresh = true;
}

void TextUIElement::Update(RenderEngine* a_renderEngine)
{
    if (m_refresh && m_fontAddr != -1)
    {
        if (m_textureAddr != -1)
        {
            m_lastRenderEngine->DestroyTexture(m_textureAddr);
        }
        if (m_samplerAddr != -1)
        {
            m_lastRenderEngine->DestroyTextureSampler(m_samplerAddr);
        }

        const Font* font = a_renderEngine->GetFont(m_fontAddr);
        if (font == nullptr)
        {
            return;
        }

        const glm::vec2 size = GetSize();

        const unsigned char* data = font->StringToTexture(m_text, m_fontSize, (uint32_t)size.x, (uint32_t)size.y);
        ICARIAN_DEFER_delA(data);

        m_textureAddr = a_renderEngine->GenerateAlphaTexture((uint32_t)size.x, (uint32_t)size.y, data);
        m_samplerAddr = a_renderEngine->GenerateTextureSampler(m_textureAddr, FlareBase::TextureMode_Texture, FlareBase::TextureFilter_Linear, FlareBase::TextureAddress_ClampToEdge);

        m_lastRenderEngine = a_renderEngine;

        m_refresh = false;
    }
}