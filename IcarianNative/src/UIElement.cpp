// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/UI/UIElement.h"
#include "IcarianError.h"

#include <glm/gtx/matrix_transform_2d.hpp>

UIElement::UIElement()
{
    m_parent = -1;
    m_childCount = 0;
    m_children = nullptr;

    m_xAnchor = UIXAnchor_Middle;
    m_yAnchor = UIYAnchor_Middle;

    m_pos = glm::vec2(0.0f);
    m_size = glm::vec2(100.0f);

    m_color = glm::vec4(1.0f);

    m_state = ElementState_Normal;
}
UIElement::~UIElement()
{
    if (m_children != nullptr)
    {
        delete[] m_children;
        m_children = nullptr;
    }
}

void UIElement::AddChild(uint32_t a_childAddr)
{
    const uint32_t* oldBuffer = m_children;
    IDEFER(
    if (oldBuffer != nullptr)
    {
        delete[] oldBuffer;
    });

    m_children = new uint32_t[m_childCount + 1];
    for (uint32_t i = 0; i < m_childCount; ++i)
    {
        m_children[i] = oldBuffer[i];
    } 

    m_children[m_childCount++] = a_childAddr;
}
void UIElement::RemoveChild(uint32_t a_childAddr)
{
    for (uint32_t i = 0; i < m_childCount; ++i) 
    {
        if (m_children[i] == a_childAddr) 
        {
            --m_childCount;

            for (uint32_t j = i; j < m_childCount; ++j) 
            {
                m_children[j] = m_children[j + 1];
            }

            return;
        }
    }
}

float UIElement::GetXPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
{
    const float scaledPos = m_pos.x / a_canvas.ReferenceResolution.x;

    switch (m_xAnchor) 
    {
    case UIXAnchor_Left:
    {
        return scaledPos;
    }
    case UIXAnchor_Middle:
    case UIXAnchor_Stretch:
    {
        const float halfSize = GetXSize(a_canvas, a_screenSize) * 0.5f;
        const float finalPos = scaledPos - halfSize;

        if (m_parent != -1)
        {
            const UIElement* parent = UIControl::GetUIElement(m_parent);

            const float parentHalf = parent->GetXSize(a_canvas, a_screenSize) * 0.5f;
            return parentHalf + finalPos;
        }

        return 0.5f + finalPos;
    }
    case UIXAnchor_Right:
    {
        const float size = GetXSize(a_canvas, a_screenSize);
        const float finalPos = scaledPos - size;

        if (m_parent != -1)
        {
            const UIElement* parent = UIControl::GetUIElement(m_parent);

            const float parentSize = parent->GetXSize(a_canvas, a_screenSize);
            return parentSize + finalPos;
        }

        return 1.0f + finalPos;
    }
    }

    IERROR("Invalid UIXAnchor");

    return 0.0f;
}
float UIElement::GetYPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
{
    const float scaledPos = m_pos.y / a_canvas.ReferenceResolution.y;

    switch (m_yAnchor)
    {
    case UIYAnchor_Top:
    {
        return scaledPos;
    }
    case UIYAnchor_Middle:
    case UIYAnchor_Stretch:
    {
        const float halfSize = GetYSize(a_canvas, a_screenSize);
        const float finalPos = scaledPos - halfSize;

        if (m_parent != -1)
        {
            const UIElement* parent = UIControl::GetUIElement(m_parent);

            const float parentHalf = parent->GetYSize(a_canvas, a_screenSize);
            return parentHalf + finalPos;
        }

        return 0.5f + finalPos;
    }
    case UIYAnchor_Bottom:
    {
        const float size = GetYSize(a_canvas, a_screenSize);
        const float finalPos = scaledPos - size;

        if (m_parent != -1)
        {
            const UIElement* parent = UIControl::GetUIElement(m_parent);

            const float parentSize = parent->GetYSize(a_canvas, a_screenSize);
            return parentSize + finalPos;
        }

        return 1.0f + finalPos;
    }
    }

    IVERIFY("Invalid UIYAnchor");

    return 0.0f;
}

float UIElement::GetXSize(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
{
    const float scaled = m_size.x / a_canvas.ReferenceResolution.x;

    switch (m_xAnchor) 
    {
    case UIXAnchor_Left:
    case UIXAnchor_Middle:
    case UIXAnchor_Right:
    {
        const float xScale = a_screenSize.x / a_canvas.ReferenceResolution.x;
        return scaled / xScale;
    }
    case UIXAnchor_Stretch:
    {
        return scaled;
    }
    }

    IERROR("Invalid UIXAnchor");

    return 0.0f;
}
float UIElement::GetYSize(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
{
    const float scaled = m_size.y / a_canvas.ReferenceResolution.y;

    switch (m_yAnchor) 
    {
    case UIYAnchor_Top:
    case UIYAnchor_Middle:
    case UIYAnchor_Bottom:
    {
        const float yScale = a_screenSize.y / a_canvas.ReferenceResolution.y;
        return scaled / yScale;
    }
    case UIYAnchor_Stretch:
    {
        return scaled;
    }
    }

    IERROR("Invalid UIYAnchor");

    return 0.0f;
}

glm::vec2 UIElement::GetCanvasPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
{
    const glm::vec2 pos = glm::vec2
    (
        GetXPosition(a_canvas, a_screenSize),
        GetYPosition(a_canvas, a_screenSize)
    );

    if (m_parent != -1)
    {
        const UIElement* parent = UIControl::GetUIElement(m_parent);
        const glm::vec2 parentPos = parent->GetCanvasPosition(a_canvas, a_screenSize);

        return parentPos + pos;
    }

    return pos;
}
glm::vec2 UIElement::GetCanvasScale(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
{
    return glm::vec2
    (
        GetXSize(a_canvas, a_screenSize),
        GetYSize(a_canvas, a_screenSize)
    );
}

void UIElement::Update(RenderEngine* a_renderEngine)
{

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