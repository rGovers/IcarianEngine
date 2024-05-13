#include "Rendering/UI/TextUIElement.h"

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
    std::unique_lock g = std::unique_lock(m_lock);

    m_fontAddr = a_addr;

    m_flags |= 0b1 << RefreshBit;
}
std::u32string TextUIElement::GetText()
{
    std::shared_lock g = std::shared_lock(m_lock);

    return m_text;
}
void TextUIElement::SetText(const std::u32string_view& a_text)
{
    std::unique_lock g = std::unique_lock(m_lock);

    m_text = std::u32string(a_text);

    m_flags |= 0b1 << RefreshBit;
}

void TextUIElement::Update(RenderEngine* a_renderEngine)
{
    // TODO: Very strange bug causes driver crash 
    // Can go a while between crashes or could be immediate
    // Application persists through driver restart but is unresponsive
    // Eliminated vulkan validation errors in runtime and still crashing driver
    // Suspect driver/gpu level memory corruption as other applications get wonky just before driver crash
    // Likely to be in here as it only happens when updating the text to my knowledge
    // NV 535.54.03 for future reference
    // Also probably want to reuse the texture and sampler if the size is the same
    // Will probably rewrite down the line anyway
    // Update: After serveral updates to drivers crash magically stopped so further suspicion of driver issue but still not confident on that so will leave this here for future reference
    // Also added lock for a different issue but may have fixed this one that disappeared after driver update but also uncertain on that
    if (m_flags & 0b1 << RefreshBit && m_fontAddr != -1)
    {
        std::shared_lock g = std::shared_lock(m_lock);

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

        m_flags &= ~(0b1 << RefreshBit);
        m_flags |= 0b1 << ValidBit;
    }
}