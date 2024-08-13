// Icarian Engine - C# Game Engine
// 
// License at end of file.

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