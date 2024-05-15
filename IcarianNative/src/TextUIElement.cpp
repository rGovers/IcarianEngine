#include "Rendering/UI/TextUIElement.h"

#include "Core/Bitfield.h"
#include "Core/IcarianAssert.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/Font.h"

TextUIElement::TextUIElement() : UIElement()
{
    m_fontAddr = -1;
    m_fontSize = 10.0f;

    m_textureAddr = -1;
    m_samplerAddr = -1;

    m_flags = 0;

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
    const ThreadGuard g = ThreadGuard(m_lock);

    m_fontAddr = a_addr;

    ISETBIT(m_flags, RefreshBit);
}
std::u32string TextUIElement::GetText()
{
    const SharedThreadGuard g = SharedThreadGuard(m_lock);

    return m_text;
}
void TextUIElement::SetText(const std::u32string_view& a_text)
{
    const ThreadGuard g = ThreadGuard(m_lock);

    m_text = std::u32string(a_text);

    ISETBIT(m_flags, RefreshBit);
}

void TextUIElement::Update(RenderEngine* a_renderEngine)
{
    if (IISBITSET(m_flags, RefreshBit) && m_fontAddr != -1)
    {
        const SharedThreadGuard g = SharedThreadGuard(m_lock);

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
        IDEFER(delete[] data);

        m_textureAddr = a_renderEngine->GenerateTexture((uint32_t)size.x, (uint32_t)size.y, TextureFormat_Alpha, data);
        m_samplerAddr = a_renderEngine->GenerateTextureSampler(m_textureAddr, TextureMode_Texture, TextureFilter_Linear, TextureAddress_ClampToEdge);

        m_lastRenderEngine = a_renderEngine;

        ICLEARBIT(m_flags, RefreshBit);
        ISETBIT(m_flags, ValidBit);
    }
}