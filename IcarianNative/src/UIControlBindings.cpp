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
    F(void, IcarianEngine.Rendering.UI, UIElement, AddChildElement, { Instance->AddElementChild(a_addr, a_childAddr); }, uint32_t a_addr, uint32_t a_childAddr) \
    F(void, IcarianEngine.Rendering.UI, UIElement, RemoveChildElement, { Instance->RemoveElementChild(a_addr, a_childAddr); }, uint32_t a_addr, uint32_t a_childAddr) \
    F(glm::vec2, IcarianEngine.Rendering.UI, UIElement, GetPosition, { return Instance->GetElementPosition(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, UIElement, SetPosition, { Instance->SetElementPosition(a_addr, a_pos); }, uint32_t a_addr, glm::vec2 a_pos) \
    F(glm::vec2, IcarianEngine.Rendering.UI, UIElement, GetSize, { return Instance->GetElementSize(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, UIElement, SetSize, { Instance->SetElementSize(a_addr, a_size); }, uint32_t a_addr, glm::vec2 a_size) \
    \
    F(uint32_t, IcarianEngine.Rendering.UI, TextUIElement, CreateTextElement, { return Instance->CreateTextElement(); }) \
    F(void, IcarianEngine.Rendering.UI, TextUIElement, DestroyTextElement, { Instance->DestroyTextElement(a_addr); }, uint32_t a_addr) \
    F(float, IcarianEngine.Rendering.UI, TextUIElement, GetFontSize, { return Instance->GetTextElementFontSize(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, TextUIElement, SetFontSize, { Instance->SetTextElementFontSize(a_addr, a_size); }, uint32_t a_addr, float a_size) \
    F(uint32_t, IcarianEngine.Rendering.UI, TextUIElement, GetFont, { return Instance->GetTextElementFont(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine.Rendering.UI, TextUIElement, SetFont, { Instance->SetTextElementFont(a_addr, a_fontAddr); }, uint32_t a_addr, uint32_t a_fontAddr) \

UICONTROL_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

RUNTIME_FUNCTION(MonoArray*, Canvas, GetChildren, 
{
    uint32_t count = 0;
    const uint32_t* children = Instance->GetCanvasChildren(a_addr, &count);

    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), count);

    for (uint32_t i = 0; i < count; ++i)
    {
        mono_array_set(arr, uint32_t, i, children[i]);
    }

    return arr;
}, uint32_t a_addr)

RUNTIME_FUNCTION(MonoArray*, UIElement, GetChildren, 
{
    uint32_t count = 0;
    const uint32_t* children = Instance->GetElementChildren(a_addr, &count);

    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_uint32_class(), count);

    for (uint32_t i = 0; i < count; ++i)
    {
        mono_array_set(arr, uint32_t, i, children[i]);
    }

    return arr;
}, uint32_t a_addr)

RUNTIME_FUNCTION(MonoString*, TextUIElement, GetText, 
{
    const std::u32string text = Instance->GetTextElementText(a_addr);

    return mono_string_new_utf32(mono_domain_get(), (const mono_unichar4*)text.c_str(), text.size());
}, uint32_t a_addr)
RUNTIME_FUNCTION(void, TextUIElement, SetText, 
{
    mono_unichar4* str = mono_string_to_utf32(a_str);
    ICARIAN_DEFER_monoF(str);

    Instance->SetTextElementText(a_addr, (char32_t*)str);
}, uint32_t a_addr, MonoString* a_str)

UIControlBindings::UIControlBindings(UIControl* a_uiControl, RuntimeManager* a_runtime)
{
    Instance = this;

    m_uiControl = a_uiControl;

    UICONTROL_BINDING_FUNCTION_TABLE(UICONTROL_RUNTIME_ATTACH);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.UI, Canvas, GetChildren);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.UI, UIElement, GetChildren);

    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.UI, TextUIElement, GetText);
    BIND_FUNCTION(a_runtime, IcarianEngine.Rendering.UI, TextUIElement, SetText);
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
uint32_t* UIControlBindings::GetCanvasChildren(uint32_t a_addr, uint32_t* a_count) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_canvas.Size(), "GetCanvasChildren out of bounds");

    const CanvasBuffer buffer = m_uiControl->m_canvas[a_addr];

    *a_count = buffer.ChildElementCount;
    return buffer.ChildElements;
}

void UIControlBindings::AddElementChild(uint32_t a_addr, uint32_t a_childAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "AddElementChild out of bounds");
    ICARIAN_ASSERT_MSG(a_childAddr < m_uiControl->m_uiElements.Size(), "AddElementChild child out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "AddElementChild element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_childAddr] != nullptr, "AddElementChild child element deleted");

    m_uiControl->m_uiElements[a_addr]->AddChild(a_childAddr);
}
void UIControlBindings::RemoveElementChild(uint32_t a_addr, uint32_t a_childAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "RemoveElementChild out of bounds");
    ICARIAN_ASSERT_MSG(a_childAddr < m_uiControl->m_uiElements.Size(), "RemoveElementChild child out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "RemoveElementChild element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_childAddr] != nullptr, "RemoveElementChild child element deleted");

    m_uiControl->m_uiElements[a_addr]->RemoveChild(a_childAddr);
}
uint32_t* UIControlBindings::GetElementChildren(uint32_t a_addr, uint32_t* a_count) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "GetElementChildren out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "GetElementChildren element deleted");
    
    *a_count = m_uiControl->m_uiElements[a_addr]->GetChildCount();
    return m_uiControl->m_uiElements[a_addr]->GetChildren();
}
glm::vec2 UIControlBindings::GetElementPosition(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "GetElementPosition out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "GetElementPosition element deleted");

    return m_uiControl->m_uiElements[a_addr]->GetPosition();
}
void UIControlBindings::SetElementPosition(uint32_t a_addr, const glm::vec2& a_pos) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "SetElementPosition out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "SetElementPosition element deleted");

    m_uiControl->m_uiElements[a_addr]->SetPosition(a_pos);
}
glm::vec2 UIControlBindings::GetElementSize(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "GetElementSize out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "GetElementSize element deleted");

    return m_uiControl->m_uiElements[a_addr]->GetSize();
}
void UIControlBindings::SetElementSize(uint32_t a_addr, const glm::vec2& a_size) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "SetElementSize out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "SetElementSize element deleted");

    m_uiControl->m_uiElements[a_addr]->SetSize(a_size);
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
std::u32string UIControlBindings::GetTextElementText(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "GetTextElementText out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "GetTextElementText element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "GetTextElementText non text element");

    const TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    return element->GetText();
}
void UIControlBindings::SetTextElementText(uint32_t a_addr, const std::u32string_view& a_text) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "SetTextElementText out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "SetTextElementText element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "SetTextElementText non text element");

    TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    element->SetText(a_text);
}
uint32_t UIControlBindings::GetTextElementFont(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "GetTextElementFont out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "GetTextElementFont element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "GetTextElementFont non text element");

    const TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    return element->GetFontAddr();
}
void UIControlBindings::SetTextElementFont(uint32_t a_addr, uint32_t a_fontAddr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "SetTextElementFont out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "SetTextElementFont element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "SetTextElementFont non text element");

    TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    element->SetFontAddr(a_fontAddr);
}
float UIControlBindings::GetTextElementFontSize(uint32_t a_addr) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "GetTextElementFontSize out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "GetTextElementFontSize element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "GetTextElementFontSize non text element");

    const TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    return element->GetFontSize();
}
void UIControlBindings::SetTextElementFontSize(uint32_t a_addr, float a_size) const
{
    ICARIAN_ASSERT_MSG(a_addr < m_uiControl->m_uiElements.Size(), "SetTextElementFontSize out of bounds");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr] != nullptr, "SetTextElementFontSize element deleted");
    ICARIAN_ASSERT_MSG(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text, "SetTextElementFontSize non text element");

    TextUIElement* element = (TextUIElement*)m_uiControl->m_uiElements[a_addr];
    element->SetFontSize(a_size);
}
