#pragma once

#include <cstdint>

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

protected:

public:
    UIElement()
    {
        m_childCount = 0;
        m_children = nullptr;
    }
    virtual ~UIElement()
    {
        if (m_children != nullptr)
        {
            delete[] m_children;
            m_children = nullptr;
        }
    }

    virtual e_UIElementType GetType() const
    {
        return UIElementType_Null;
    }

    virtual void Update(RenderEngine* a_renderEngine) = 0;
};