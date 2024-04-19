#include "Rendering/UI/UIControl.h"

#include "Core/IcarianAssert.h"
#include "Logger.h"
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControlBindings.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

UIControl* UIControl::Instance = nullptr;

UIControl::UIControl()
{
    m_bindings = new UIControlBindings(this);

    m_onNormal = RuntimeManager::GetFunction("IcarianEngine.Rendering.UI", "UIElement", ":OnNormalS(uint,uint)");
    m_onHover = RuntimeManager::GetFunction("IcarianEngine.Rendering.UI", "UIElement", ":OnHoverS(uint,uint)");
    m_onPressed = RuntimeManager::GetFunction("IcarianEngine.Rendering.UI", "UIElement", ":OnPressedS(uint,uint)");
    m_onReleased = RuntimeManager::GetFunction("IcarianEngine.Rendering.UI", "UIElement", ":OnReleasedS(uint,uint)");
}
UIControl::~UIControl()
{
    delete m_onNormal;
    delete m_onHover;
    delete m_onPressed;
    delete m_onReleased;

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

void UIControl::Init()
{
    if (Instance == nullptr)
    {
        Instance = new UIControl();
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

void UIControl::SendCursor(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_scaledPos, const glm::vec2& a_scale)
{
    ICARIAN_ASSERT_MSG(a_canvasAddr < Instance->m_canvas.Size(), "SendCursor canvas out of bounds");
    ICARIAN_ASSERT_MSG(Instance->m_canvas[a_canvasAddr].IsDestroyed() == false, "SendCursor canvas is destroyed");
    ICARIAN_ASSERT_MSG(a_elementAddr < Instance->m_uiElements.Size(), "SendCursor UIElement out of bounds");
    ICARIAN_ASSERT_MSG(Instance->m_uiElements[a_elementAddr] != nullptr, "SendCursor UIElement is null");

    UIElement* element = Instance->m_uiElements[a_elementAddr];    
    const CanvasBuffer& canvas = Instance->m_canvas[a_canvasAddr];

    const glm::vec2 pos = element->GetCanvasPosition(canvas, canvas.ReferenceResolution);
    const glm::vec2 size = element->GetCanvasScale(canvas, canvas.ReferenceResolution) / a_scale;

    const glm::vec2 endPos = pos + size;

    if (a_scaledPos.x >= pos.x && a_scaledPos.x <= endPos.x &&
        a_scaledPos.y >= pos.y && a_scaledPos.y <= endPos.y)
    {
        if (element->GetState() == ElementState_Normal)
        {
            element->SetState(ElementState_Hovered);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onHover->Exec(args);
        }
    }
    else if (element->GetState() == ElementState_Hovered)
    {
        element->SetState(ElementState_Normal);

        void* args[] = { &a_canvasAddr, &a_elementAddr };

        Instance->m_onNormal->Exec(args);
    }

    const uint32_t childCount = element->GetChildCount();
    const uint32_t* children = element->GetChildren();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        if (children[i] < 0)
        {
            continue;
        }

        Instance->SendCursor(a_canvasAddr, children[i], a_scaledPos, a_scale);
    }
}

bool UIControl::SendClick(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_scaledPos, const glm::vec2& a_scale)
{
    ICARIAN_ASSERT_MSG(a_canvasAddr < Instance->m_canvas.Size(), "SendClick canvas out of bounds");
    ICARIAN_ASSERT_MSG(Instance->m_canvas[a_canvasAddr].IsDestroyed() == false, "SendClick canvas is destroyed");
    ICARIAN_ASSERT_MSG(a_elementAddr < Instance->m_uiElements.Size(), "SendClick UIElement out of bounds");
    ICARIAN_ASSERT_MSG(Instance->m_uiElements[a_elementAddr] != nullptr, "SendClick UIElement is null");

    UIElement* element = Instance->m_uiElements[a_elementAddr];
    const CanvasBuffer& canvas = Instance->m_canvas[a_canvasAddr];

    const glm::vec2 pos = element->GetCanvasPosition(canvas, canvas.ReferenceResolution);
    const glm::vec2 size = element->GetCanvasScale(canvas, canvas.ReferenceResolution) / a_scale;

    const glm::vec2 endPos = pos + size;

    if (a_scaledPos.x >= pos.x && a_scaledPos.x <= endPos.x &&
        a_scaledPos.y >= pos.y && a_scaledPos.y <= endPos.y)
    {
        if (element->GetState() != ElementState_Pressed)
        {
            element->SetState(ElementState_Pressed);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onPressed->Exec(args);
        }
        
        return true;
    }
    else 
    {
        switch (element->GetState())
        {
        case ElementState_Pressed:
        case ElementState_Released:
        {
            element->SetState(ElementState_Normal);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onReleased->Exec(args);

            break;
        }
        }
    }

    const uint32_t childCount = element->GetChildCount();
    const uint32_t* children = element->GetChildren();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        if (children[i] < 0)
        {
            continue;
        }

        if (Instance->SendClick(a_canvasAddr, children[i], a_scaledPos, a_scale))
        {
            return true;
        }
    }

    return false;
}

void UIControl::SendRelease(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_scaledPos, const glm::vec2& a_scale)
{
    ICARIAN_ASSERT_MSG(a_canvasAddr < Instance->m_canvas.Size(), "SendRelease canvas out of bounds");
    ICARIAN_ASSERT_MSG(Instance->m_canvas[a_canvasAddr].IsDestroyed() == false, "SendRelease canvas is destroyed");
    ICARIAN_ASSERT_MSG(a_elementAddr < Instance->m_uiElements.Size(), "SendRelease UIElement out of bounds");
    ICARIAN_ASSERT_MSG(Instance->m_uiElements[a_elementAddr] != nullptr, "SendRelease UIElement is null");

    UIElement* element = Instance->m_uiElements[a_elementAddr];
    const CanvasBuffer& canvas = Instance->m_canvas[a_canvasAddr];

    const glm::vec2 pos = element->GetCanvasPosition(canvas, canvas.ReferenceResolution);
    const glm::vec2 size = element->GetCanvasScale(canvas, canvas.ReferenceResolution) / a_scale;

    const glm::vec2 endPos = pos + size;

    if (a_scaledPos.x >= pos.x && a_scaledPos.x <= endPos.x &&
        a_scaledPos.y >= pos.y && a_scaledPos.y <= endPos.y)
    {
        switch (element->GetState())
        {
        case ElementState_Pressed:
        {
            element->SetState(ElementState_Released);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onReleased->Exec(args);

            break;
        }
        case ElementState_Released:
        {
            element->SetState(ElementState_Hovered);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onHover->Exec(args);

            break;
        }
        }
    }
    else if (element->GetState() == ElementState_Released)
    {
        element->SetState(ElementState_Normal);

        void* args[] = { &a_canvasAddr, &a_elementAddr };

        Instance->m_onReleased->Exec(args);
    }

    const uint32_t childCount = element->GetChildCount();
    const uint32_t* children = element->GetChildren();
    for (uint32_t i = 0; i < childCount; ++i)
    {
        if (children[i] < 0)
        {
            continue;
        }

        Instance->SendRelease(a_canvasAddr, children[i], a_scaledPos, a_scale);
    }
}

void UIControl::UpdateCursor(const glm::vec2& a_pos, const glm::vec2& a_size)
{
    // Not modifying the canvas directly but do not want it to be modified while
    // we doing this
    TReadLockArray<CanvasBuffer> a = Instance->m_canvas.ToReadLockArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i) 
    {
        const CanvasBuffer& canvas = a[i];

        if (canvas.IsDestroyed() || !canvas.CaptureInput()) 
        {
            continue;
        }

        // Need to get from screen space to canvas space
        const glm::vec2 scale = a_size / canvas.ReferenceResolution;
        const glm::vec2 sPos = a_pos / a_size;

        const uint32_t childCount = canvas.ChildElementCount;
        const uint32_t* children = canvas.ChildElements;
        for (uint32_t j = 0; j < childCount; ++j) 
        {
            if (children[j] < 0) 
            {
                continue;
            }

            Instance->SendCursor(i, children[j], sPos, scale);
        }
    }
}

bool UIControl::SubmitClick(const glm::vec2& a_pos, const glm::vec2& a_size)
{
    // Not modifying the canvas directly but do not want it to be modified while
    // we doing this
    TReadLockArray<CanvasBuffer> a = Instance->m_canvas.ToReadLockArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        const CanvasBuffer& canvas = a[i];

        if (canvas.IsDestroyed() || !canvas.CaptureInput())
        {
            continue;
        }

        // Need to get from screen space to canvas space
        const glm::vec2 scale = a_size / canvas.ReferenceResolution;
        const glm::vec2 sPos = a_pos / a_size;

        const uint32_t childCount = canvas.ChildElementCount;
        const uint32_t* children = canvas.ChildElements;
        for (uint32_t j = 0; j < childCount; ++j)
        {
            if (children[j] < 0)
            {
                continue;
            }

            if (Instance->SendClick(i, children[j], sPos, scale))
            {
                return true;
            }
        }
    }

    return false;
}

void UIControl::SubmitRelease(const glm::vec2& a_pos, const glm::vec2& a_size)
{
    // Not modifying the canvas directly but do not want it to be modified while
    // we doing this
    TReadLockArray<CanvasBuffer> a = Instance->m_canvas.ToReadLockArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        const CanvasBuffer& canvas = a[i];

        if (canvas.IsDestroyed() || !canvas.CaptureInput())
        {
            continue;
        }

        // Need to get from screen space to canvas space
        const glm::vec2 scale = a_size / canvas.ReferenceResolution;
        const glm::vec2 sPos = a_pos / a_size;

        const uint32_t childCount = canvas.ChildElementCount;
        const uint32_t* children = canvas.ChildElements;
        for (uint32_t j = 0; j < childCount; ++j)
        {
            if (children[j] < 0)
            {
                continue;
            }

            Instance->SendRelease(i, children[j], sPos, scale);
        }
    }
}