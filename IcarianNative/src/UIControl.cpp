#include "Rendering/UI/UIControl.h"

#include "Flare/IcarianAssert.h"
#include "Logger.h"
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControlBindings.h"
#include "Trace.h"

UIControl* UIControl::Instance = nullptr;

UIControl::UIControl(RuntimeManager* a_runtime)
{
    m_bindings = new UIControlBindings(this, a_runtime);
}
UIControl::~UIControl()
{
    delete m_bindings;

    for (uint32_t i = 0; i < m_canvas.Size(); ++i)
    {
        if (!(m_canvas[i].Flags & 0b1 << CanvasBuffer::DestroyedBit))
        {
            Logger::Warning("CanvasBuffer was not deleted");
        }

        if (m_canvas[i].ChildElements != nullptr)
        {
            Logger::Warning("CanvasBuffer ChildElements was not deleted");

            delete[] m_canvas[i].ChildElements;
        }
    }

    for (uint32_t i = 0; i < m_uiElements.Size(); ++i)
    {
        if (m_uiElements[i] != nullptr)
        {
            Logger::Warning("UIElement was not deleted");

            switch (m_uiElements[i]->GetType()) 
            {
            case UIElementType_Text:
            {
                delete (TextUIElement*)m_uiElements[i];

                break;
            }
            default:
            {
                ICARIAN_ASSERT_MSG(0, "Unknown UIElementType");

                break;   
            }
            }
        }
    }
}

void UIControl::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new UIControl(a_runtime);
    }
}
void UIControl::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

std::vector<CanvasBuffer> UIControl::GetCanvases()
{
    return Instance->m_canvas.ToVector();
}

CanvasBuffer UIControl::GetCanvas(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_canvas.Size(), "GetCanvas out of bounds");

    return Instance->m_canvas[a_addr];
}
void UIControl::SetCanvas(uint32_t a_addr, const CanvasBuffer& a_buffer)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_canvas.Size(), "SetCanvas out of bounds");

    Instance->m_canvas.LockSet(a_addr, a_buffer);
}

UIElement* UIControl::GetUIElement(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_uiElements.Size(), "GetUIElement out of bounds");

    return Instance->m_uiElements[a_addr];
}