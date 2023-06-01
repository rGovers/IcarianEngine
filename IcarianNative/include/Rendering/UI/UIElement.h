#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Flare/IcarianDefer.h"

enum e_UIElementType : uint32_t
{
    UIElementType_Null,
    UIElementType_Text
};

class RenderEngine;

class UIElement
{
private:
    uint32_t* m_children;
    uint32_t  m_childCount;

    glm::vec2 m_pos;
    glm::vec2 m_size;

protected:

public:
    UIElement()
    {
        m_childCount = 0;
        m_children = nullptr;

        m_pos = glm::vec2(0.0f);
        m_size = glm::vec2(100.0f);
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