#include "Rendering/UI/UIControl.h"

#include "Core/Bitfield.h"
#include "IcarianError.h"
#include "Logger.h"
#include "Rendering/UI/UIControlBindings.h"
#include "Rendering/UI/UIElement.h"
#include "Runtime/RuntimeFunction.h"
#include "Runtime/RuntimeManager.h"

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
        if (!m_canvas.Exists(i))
        {
            continue;
        }

        if (m_canvas[i].ChildElements != nullptr)
        {
            Logger::Warning("CanvasBuffer ChildElements was not deleted");

            delete[] m_canvas[i].ChildElements;
        }
    }

    for (uint32_t i = 0; i < m_uiElements.Size(); ++i)
    {
        if (m_uiElements.Exists(i))
        {
            Logger::Warning("UIElement was not deleted");

            delete m_uiElements[i];
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
    IVERIFY(a_addr < Instance->m_canvas.Size());
    IVERIFY(Instance->m_canvas.Exists(a_addr));

    return Instance->m_canvas[a_addr];
}
void UIControl::SetCanvas(uint32_t a_addr, const CanvasBuffer& a_buffer)
{
    IVERIFY(a_addr < Instance->m_canvas.Size());
    IVERIFY(Instance->m_canvas.Exists(a_addr));

    Instance->m_canvas.LockSet(a_addr, a_buffer);
}

UIElement* UIControl::GetUIElement(uint32_t a_addr)
{
    IVERIFY(a_addr < Instance->m_uiElements.Size());
    IVERIFY(Instance->m_uiElements.Exists(a_addr));

    return Instance->m_uiElements[a_addr];
}

static bool IsInside(const CanvasBuffer& a_canvas, const UIElement* a_element, const glm::vec2& a_pos, const glm::vec2& a_screenSize)
{
    const glm::vec2 pos = a_element->GetCanvasPosition(a_canvas, a_screenSize);
    const glm::vec2 size = a_element->GetCanvasScale(a_canvas, a_screenSize);

    const glm::vec2 scaledPos = pos * a_screenSize;
    const glm::vec2 scaledSize = size * a_screenSize;

    const glm::vec2 endPos = scaledPos + scaledSize;

    return a_pos.x >= scaledPos.x && a_pos.x <= endPos.x &&
        a_pos.y >= scaledPos.y && a_pos.y <= endPos.y;
}

void UIControl::SendCursor(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_pos, const glm::vec2& a_screenSize)
{
    IVERIFY(a_canvasAddr < Instance->m_canvas.Size());
    IVERIFY(Instance->m_canvas.Exists(a_canvasAddr));
    IVERIFY(a_elementAddr < Instance->m_uiElements.Size());
    IVERIFY(Instance->m_uiElements.Exists(a_elementAddr));

    const CanvasBuffer& canvas = Instance->m_canvas[a_canvasAddr];
    UIElement* element = Instance->m_uiElements[a_elementAddr];    

    const e_ElementState state = element->GetState();

    if (IsInside(canvas, element, a_pos, a_screenSize))
    {
        if (state == ElementState_Normal)
        {
            element->SetState(ElementState_Hovered);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onHover->Exec(args);
        }
    }
    else if (state == ElementState_Hovered)
    {
        element->SetState(ElementState_Normal);

        void* args[] = 
        { 
            &a_canvasAddr, 
            &a_elementAddr 
        };

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

        Instance->SendCursor(a_canvasAddr, children[i], a_pos, a_screenSize);
    }
}

bool UIControl::SendClick(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_pos, const glm::vec2& a_screenSize)
{
    IVERIFY(a_canvasAddr < Instance->m_canvas.Size());
    IVERIFY(Instance->m_canvas.Exists(a_canvasAddr));
    IVERIFY(a_elementAddr < Instance->m_uiElements.Size());
    IVERIFY(Instance->m_uiElements.Exists(a_elementAddr));

    const CanvasBuffer& canvas = Instance->m_canvas[a_canvasAddr];
    UIElement* element = Instance->m_uiElements[a_elementAddr];

    const e_ElementState state = element->GetState();

    if (IsInside(canvas, element, a_pos, a_screenSize))
    {
        if (state != ElementState_Pressed)
        {
            element->SetState(ElementState_Pressed);

            void* args[] = { &a_canvasAddr, &a_elementAddr };

            Instance->m_onPressed->Exec(args);
        }
        
        return true;
    }
    else 
    {
        switch (state)
        {
        case ElementState_Pressed:
        case ElementState_Released:
        {
            element->SetState(ElementState_Normal);

            void* args[] = 
            { 
                &a_canvasAddr, 
                &a_elementAddr 
            };

            Instance->m_onNormal->Exec(args);

            break;
        }
        default:
        {
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

        if (Instance->SendClick(a_canvasAddr, children[i], a_pos, a_screenSize))
        {
            return true;
        }
    }

    return false;
}

void UIControl::SendRelease(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_pos, const glm::vec2& a_screenSize)
{
    IVERIFY(a_canvasAddr < Instance->m_canvas.Size());
    IVERIFY(Instance->m_canvas.Exists(a_canvasAddr));
    IVERIFY(a_elementAddr < Instance->m_uiElements.Size());
    IVERIFY(Instance->m_uiElements.Exists(a_elementAddr));

    const CanvasBuffer& canvas = Instance->m_canvas[a_canvasAddr];
    UIElement* element = Instance->m_uiElements[a_elementAddr];

    const e_ElementState state = element->GetState();

    if (IsInside(canvas, element, a_pos, a_screenSize))
    {
        switch (state)
        {
        case ElementState_Pressed:
        {
            element->SetState(ElementState_Released);

            void* args[] = 
            { 
                &a_canvasAddr, 
                &a_elementAddr 
            };

            Instance->m_onReleased->Exec(args);

            break;
        }
        case ElementState_Released:
        {
            element->SetState(ElementState_Hovered);

            void* args[] = 
            { 
                &a_canvasAddr, 
                &a_elementAddr 
            };

            Instance->m_onHover->Exec(args);

            break;
        }
        default:
        {
            break;
        }
        }
    }
    else if (state == ElementState_Released)
    {
        element->SetState(ElementState_Normal);

        void* args[] = 
        { 
            &a_canvasAddr, 
            &a_elementAddr 
        };

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

        Instance->SendRelease(a_canvasAddr, children[i], a_pos, a_screenSize);
    }
}

void UIControl::UpdateCursor(const glm::vec2& a_pos, const glm::vec2& a_size)
{
    // Not modifying the canvas directly but do not want it to be modified while
    // we are doing this
    const TReadLockArray<CanvasBuffer> a = Instance->m_canvas.ToReadLockArray();
    const Array<bool> state = Instance->m_canvas.ToStateArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i) 
    {
        if (!state[i])
        {
            continue;
        }

        const CanvasBuffer& canvas = a[i];
        if (!IISBITSET(canvas.Flags, CanvasBuffer::CaptureInputBit)) 
        {
            continue;
        }

        const uint32_t childCount = canvas.ChildCount;
        const uint32_t* children = canvas.ChildElements;
        for (uint32_t j = 0; j < childCount; ++j) 
        {
            if (children[j] < 0) 
            {
                continue;
            }

            Instance->SendCursor(i, children[j], a_pos, a_size);
        }
    }
}

bool UIControl::SubmitClick(const glm::vec2& a_pos, const glm::vec2& a_size)
{
    // Not modifying the canvas directly but do not want it to be modified while
    // we are doing this
    const TReadLockArray<CanvasBuffer> a = Instance->m_canvas.ToReadLockArray();
    const Array<bool> state = Instance->m_canvas.ToStateArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        const CanvasBuffer& canvas = a[i];
        if (!IISBITSET(canvas.Flags, CanvasBuffer::CaptureInputBit))
        {
            continue;
        }

        const uint32_t childCount = canvas.ChildCount;
        const uint32_t* children = canvas.ChildElements;
        for (uint32_t j = 0; j < childCount; ++j)
        {
            if (children[j] < 0)
            {
                continue;
            }

            if (Instance->SendClick(i, children[j], a_pos, a_size))
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
    const TReadLockArray<CanvasBuffer> a = Instance->m_canvas.ToReadLockArray();
    const Array<bool> state = Instance->m_canvas.ToStateArray();

    const uint32_t size = a.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (!state[i])
        {
            continue;
        }

        const CanvasBuffer& canvas = a[i];
        if (!IISBITSET(canvas.Flags, CanvasBuffer::CaptureInputBit))
        {
            continue;
        }

        const uint32_t childCount = canvas.ChildCount;
        const uint32_t* children = canvas.ChildElements;
        for (uint32_t j = 0; j < childCount; ++j)
        {
            if (children[j] < 0)
            {
                continue;
            }

            Instance->SendRelease(i, children[j], a_pos, a_size);
        }
    }
}