#include "Rendering/UI/UIControlBindings.h"

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Rendering/UI/CanvasBuffer.h"
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControl.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static UIControlBindings* Instance = nullptr;

#define UICONTROL_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(a_runtime, namespace, klass, name);

#define UICONTROL_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.UI, Canvas, CreateCanvas, { return Instance->CreateCanvas(a_refRes); }, glm::vec2 a_refRes) \
    F(void, IcarianEngine.Rendering.UI, Canvas, DestroyCanvas, { Instance->DestroyCanvas(a_addr); }, uint32_t a_addr) \
    F(CanvasBuffer, IcarianEngine.Rendering.UI, Canvas, GetBuffer, { return UIControl::GetCanvas(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, Canvas, SetBuffer, { UIControl::SetCanvas(a_addr, a_buffer); }, uint32_t a_addr, CanvasBuffer a_buffer) \
    F(void, IcarianEngine.Rendering.UI, Canvas, AddChildElement, { Instance->AddCanvasChild(a_addr, a_uiElementAddr); }, uint32_t a_addr, uint32_t a_uiElementAddr) \
    F(void, IcarianEngine.Rendering.UI, Canvas, RemoveChildElement, { Instance->RemoveCanvasChild(a_addr, a_uiElementAddr); }, uint32_t a_addr, uint32_t a_uiElementAddr) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, TextUIElement, CreateTextElement, { return Instance->CreateTextElement(); }) \
    F(void, IcarianEngine.Rendering.UI, TextUIElement, DestroyTextElement, { Instance->DestroyTextElement(a_addr); }, uint32_t a_addr) \

UICONTROL_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

UIControlBindings::UIControlBindings(UIControl* a_uiControl, RuntimeManager* a_runtime)
{
    Instance = this;

    m_uiControl = a_uiControl;

    UICONTROL_BINDING_FUNCTION_TABLE(UICONTROL_RUNTIME_ATTACH);
}
UIControlBindings::~UIControlBindings()
{

}

uint32_t UIControlBindings::CreateCanvas(const glm::vec2& a_refResolution) const
{
    TRACE("Creating Canvas");

    CanvasBuffer buffer;
    buffer.ReferenceResolution = a_refResolution;
    buffer.ChildElementCount = 0;
    buffer.ChildElements = nullptr;
    buffer.Flags = 0;

    {
        TLockArray<CanvasBuffer> a = m_uiControl->m_canvas.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].Flags & 0b1 << CanvasBuffer::DestroyedBit)
            {
                a[i] = buffer;

                return i;
            }
        }
    }

    return m_uiControl->m_canvas.PushVal(buffer);
}
void UIControlBindings::DestroyCanvas(uint32_t a_addr) const
{
    TRACE("Destroying Canvas");

    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_canvas.Size(), "RemoveCanvas out of bounds");

    CanvasBuffer destroyedBuffer;
    destroyedBuffer.ChildElementCount = 0;
    destroyedBuffer.ChildElements = nullptr;
    destroyedBuffer.Flags |= 0b1 << CanvasBuffer::DestroyedBit;

    const CanvasBuffer buffer = m_uiControl->m_canvas[a_addr];
    m_uiControl->m_canvas.LockSet(a_addr, destroyedBuffer);

    if (buffer.ChildElements != nullptr)
    {
        delete[] buffer.ChildElements;
    }
}
void UIControlBindings::AddCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_canvas.Size(), "AddCanvasChild Canvas out of bounds");
    ICARIAN_ASSERT_MSG(a_uiElementAddr < m_uiControl->m_uiElements.Size(), "AddCanvasChild UIElement out of bounds");

    CanvasBuffer buffer = m_uiControl->m_canvas[a_addr];
    if (buffer.ChildElements != nullptr)
    {
        for (uint32_t i = 0; i < buffer.ChildElementCount; ++i)
        {
            if (buffer.ChildElements[i] == a_uiElementAddr)
            {
                return;
            }

            if (buffer.ChildElements[i] == -1)
            {
                buffer.ChildElements[i] = a_uiElementAddr;

                return;
            }
        }
    }

    uint32_t* newBuffer = new uint32_t[buffer.ChildElementCount + 1];
    const uint32_t* oldBuffer = buffer.ChildElements;
    ICARIAN_DEFER_delA(oldBuffer);

    for (uint32_t i = 0; i < buffer.ChildElementCount; ++i)
    {
        newBuffer[i] = oldBuffer[i];
    }

    newBuffer[buffer.ChildElementCount++] = a_uiElementAddr;
    buffer.ChildElements = newBuffer;

    m_uiControl->m_canvas.LockSet(a_addr, buffer);    
}
void UIControlBindings::RemoveCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_canvas.Size(), "RemoveCanvasChild Canvas out of bounds");
    ICARIAN_ASSERT_MSG(a_uiElementAddr < m_uiControl->m_uiElements.Size(), "RemoveCanvasChild UIElement out of bounds");

    const CanvasBuffer buffer = m_uiControl->m_canvas[a_addr];

    for (uint32_t i = 0; i < buffer.ChildElementCount; ++i)
    {
        if (buffer.ChildElements[i] == a_uiElementAddr)
        {
            buffer.ChildElements[i] = -1;

            return;
        }
    }
}

uint32_t UIControlBindings::CreateTextElement() const
{
    TRACE("Creating Text UI Element");

    TextUIElement* element = new TextUIElement();

    {
        TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = element;

                return i;
            }
        }
    }

    return m_uiControl->m_uiElements.PushVal(element);
}
void UIControlBindings::DestroyTextElement(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "DestroyTextElement out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "DestroyTextElement already destroyed");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "DestroyTextElement non text element");

    const TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    m_uiControl->m_uiElements.LockSet(a_addr, nullptr);

    delete element;
}