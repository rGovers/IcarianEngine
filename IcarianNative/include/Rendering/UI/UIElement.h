// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Rendering/UI/UIControl.h"

class RenderEngine;

#include "EngineUIElementInteropStuctures.h"

enum e_UIElementType : uint16_t
{
    UIElementType_Base,
    UIElementType_Text,
    UIElementType_Image
};

class UIElement
{
private:
    uint32_t*      m_children;
    uint32_t       m_childCount;

    uint32_t       m_parent;

    glm::vec2      m_pos;
    glm::vec2      m_size;
    glm::vec4      m_color;

    e_UIXAnchor    m_xAnchor;
    e_UIYAnchor    m_yAnchor;

    e_ElementState m_state;

    float GetXPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const;
    float GetYPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const;

    float GetXSize(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const;
    float GetYSize(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const;

protected:

public:
    UIElement();
    virtual ~UIElement();

    inline e_ElementState GetState() const
    {
        return m_state;
    }
    inline void SetState(e_ElementState a_state)
    {
        m_state = a_state;
    }

    inline e_UIXAnchor GetXAnchor() const
    {
        return m_xAnchor;
    }
    inline void SetXAnchor(e_UIXAnchor a_anchor)
    {
        m_xAnchor = a_anchor;
    }
    inline e_UIYAnchor GetYAnchor() const
    {
        return m_yAnchor;
    }
    inline void SetYAnchor(e_UIYAnchor a_anchor) 
    {
        m_yAnchor = a_anchor;
    }

    inline glm::vec2 GetPosition() const
    {
        return m_pos;
    }
    inline void SetPosition(const glm::vec2& a_pos)
    {
        m_pos = a_pos;
    }
    
    inline glm::vec2 GetSize() const
    {
        return m_size;
    }
    inline void SetSize(const glm::vec2& a_size)
    {
        m_size = a_size;
    }

    inline glm::vec4 GetColor() const
    {
        return m_color;
    }
    inline void SetColor(const glm::vec4& a_color)
    {
        m_color = a_color;
    }

    inline uint32_t GetParent() const
    {
        return m_parent;
    }
    // Not the best but not exposed to C# so it's fine for now
    inline void SetParent(uint32_t a_parent)
    {
        m_parent = a_parent;
    }

    void AddChild(uint32_t a_childAddr);
    void RemoveChild(uint32_t a_childAddr);

    glm::vec2 GetCanvasPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const;
    glm::vec2 GetCanvasScale(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const;

    inline uint32_t GetChildCount() const
    {
        return m_childCount;
    }
    inline uint32_t* GetChildren() const
    {
        return m_children;
    }

    virtual e_UIElementType GetType() const
    {
        return UIElementType_Base;
    }

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