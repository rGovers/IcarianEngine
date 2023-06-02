#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <glm/gtx/matrix_transform_2d.hpp>

#include "Flare/IcarianDefer.h"
#include "Rendering/UI/UIControl.h"
#include "Rendering/UI/CanvasBuffer.h"

enum e_UIElementType : uint32_t
{
    UIElementType_Null,
    UIElementType_Text
};

class RenderEngine;

class UIElement
{
private:
    uint32_t  m_parent;

    uint32_t* m_children;
    uint32_t  m_childCount;

    glm::vec2 m_pos;
    glm::vec2 m_size;

    glm::vec4 m_color;

    glm::mat3 GetMatrix(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
    {
        if (m_parent != -1)
        {
            const glm::mat3 transform = glm::translate(glm::mat3(1.0f), m_pos / a_canvas.ReferenceResolution);

            UIElement* parent = UIControl::GetUIElement(m_parent);

            return parent->GetMatrix(a_canvas, a_screenSize) * transform;
        }
        else 
        {
            const glm::vec2 scale = a_screenSize / a_canvas.ReferenceResolution;

            return glm::translate(glm::mat3(1.0f), m_pos / a_canvas.ReferenceResolution * scale);
        }

        return glm::mat3(1.0f);
    }

protected:

public:
    UIElement()
    {
        m_parent = -1;
        m_childCount = 0;
        m_children = nullptr;

        m_pos = glm::vec2(0.0f);
        m_size = glm::vec2(100.0f);

        m_color = glm::vec4(1.0f);
    }
    virtual ~UIElement()
    {
        if (m_children != nullptr)
        {
            delete[] m_children;
            m_children = nullptr;
        }
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

    void AddChild(uint32_t a_childAddr)
    {
        uint32_t* newBuffer = new uint32_t[m_childCount + 1];

        for (uint32_t i = 0; i < m_childCount; ++i)
        {
            newBuffer[i] = m_children[i];
        } 

        newBuffer[m_childCount++] = a_childAddr;

        ICARIAN_DEFER(m_children, if (m_children != nullptr) { delete[] m_children; });
        m_children = newBuffer;
    }
    void RemoveChild(uint32_t a_childAddr)
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

    glm::vec2 GetCanvasPosition(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
    {
        if (m_parent != -1)
        {
            const glm::mat3 transform = glm::translate(glm::mat3(1.0f), m_pos / a_canvas.ReferenceResolution);

            const UIElement* parent = UIControl::GetUIElement(m_parent);

            return (parent->GetMatrix(a_canvas, a_screenSize) * transform)[2].xy();
        }
        else 
        {
            const glm::vec2 scale = a_screenSize / a_canvas.ReferenceResolution;

            return m_pos / a_canvas.ReferenceResolution * scale;
        }

        return glm::vec2(0);
    }
    glm::vec2 GetCanvasScale(const CanvasBuffer& a_canvas, const glm::vec2& a_screenSize) const
    {
        const glm::vec2 scale = a_screenSize / a_canvas.ReferenceResolution;

        return m_size / a_canvas.ReferenceResolution;
    }

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
        return UIElementType_Null;
    }

    virtual void Update(RenderEngine* a_renderEngine) = 0;
};