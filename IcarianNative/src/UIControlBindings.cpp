// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/UI/UIControlBindings.h"

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include "Core/IcarianDefer.h"
#include "DataTypes/Allocators/MallocAllocator.h"
#include "IcarianError.h"
#include "Rendering/UI/ImageUIElement.h"
#include "Rendering/UI/TextUIElement.h"
#include "Rendering/UI/UIControl.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static UIControlBindings* Instance = nullptr;

#include "EngineCanvasInterop.h"
#include "EngineUIElementInterop.h"
#include "EngineImageUIElementInterop.h"
#include "EngineTextUIElementInterop.h"

ENGINE_CANVAS_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_UIELEMENT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_IMAGEUIELEMENT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);
ENGINE_TEXTUIELEMENT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

UIControlBindings::UIControlBindings(UIControl* a_uiControl)
{
    Instance = this;

    m_uiControl = a_uiControl;

    ENGINE_CANVAS_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_UIELEMENT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_IMAGEUIELEMENT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
    ENGINE_TEXTUIELEMENT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
UIControlBindings::~UIControlBindings()
{

}

uint32_t UIControlBindings::CreateCanvas(const glm::vec2& a_refResolution) const
{
    TRACE("Creating Canvas");
    const CanvasBuffer buffer =
    {
        .ReferenceResolution = a_refResolution,
    };

    return m_uiControl->m_canvas.PushVal(buffer);
}
void UIControlBindings::DestroyCanvas(uint32_t a_addr) const
{
    TRACE("Destroying Canvas");
    IVERIFY(m_uiControl->m_canvas.Exists(a_addr));

    const CanvasBuffer buffer = m_uiControl->m_canvas[a_addr];
    IDEFER(MallocAllocator::Instance->Free(buffer.ChildElements));

    m_uiControl->m_canvas.Erase(a_addr);
}
void UIControlBindings::AddCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const
{
    IVERIFY(m_uiControl->m_canvas.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements.Exists(a_uiElementAddr));

    {
        TLockArray<CanvasBuffer> a = m_uiControl->m_canvas.ToLockArray();

        CanvasBuffer& buffer = a[a_addr];
        if (buffer.ChildElements != nullptr)
        {
            for (uint32_t i = 0; i < buffer.ChildCount; ++i)
            {
                if (buffer.ChildElements[i] == a_uiElementAddr)
                {
                    return;
                }

                if (buffer.ChildElements[i] == uint32_t(-1))
                {
                    buffer.ChildElements[i] = a_uiElementAddr;

                    return;
                }
            }
        }

        uint32_t* oldBuffer = buffer.ChildElements;
        IDEFER(MallocAllocator::Instance->Free(oldBuffer));
        buffer.ChildElements = MallocAllocator::Instance->TAllocate<uint32_t>(buffer.ChildCount + 1);

        for (uint32_t i = 0; i < buffer.ChildCount; ++i)
        {
            buffer.ChildElements[i] = oldBuffer[i];
        }

        buffer.ChildElements[buffer.ChildCount++] = a_uiElementAddr;
    }

    {
        TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

        UIElement* element = a[a_uiElementAddr];
        element->SetParent(-1);
    }
}
void UIControlBindings::RemoveCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const
{
    IVERIFY(m_uiControl->m_canvas.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements.Exists(a_uiElementAddr));

    {
        TLockArray<CanvasBuffer> a = m_uiControl->m_canvas.ToLockArray();

        const CanvasBuffer buffer = a[a_addr];
        for (uint32_t i = 0; i < buffer.ChildCount; ++i)
        {
            if (buffer.ChildElements[i] == a_uiElementAddr)
            {
                buffer.ChildElements[i] = -1;

                break;
            }
        }
    }
}
uint32_t* UIControlBindings::GetCanvasChildren(uint32_t a_addr, uint32_t* a_count) const
{
    IVERIFY(a_addr < m_uiControl->m_canvas.Size());

    const TReadLockArray<CanvasBuffer> a = m_uiControl->m_canvas.ToReadLockArray();

    const CanvasBuffer buffer = a[a_addr];

    *a_count = buffer.ChildCount;
    return buffer.ChildElements;
}

uint32_t UIControlBindings::CreateUIElement() const
{
    UIElement* element = MallocAllocator::Instance->Create<UIElement>(MallocAllocator::Instance);

    return m_uiControl->m_uiElements.PushVal(element);
}
void UIControlBindings::DestroyUIElement(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    UIElement* element = m_uiControl->m_uiElements[a_addr];
    IDEFER(MallocAllocator::Instance->Destroy(element));
    m_uiControl->m_uiElements.Erase(a_addr);
}
void UIControlBindings::AddElementChild(uint32_t a_addr, uint32_t a_childAddr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements.Exists(a_childAddr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* pElement = a[a_addr];
    pElement->AddChild(a_childAddr);

    UIElement* cElement = a[a_childAddr];
    cElement->SetParent(a_addr);
}
void UIControlBindings::RemoveElementChild(uint32_t a_addr, uint32_t a_childAddr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements.Exists(a_childAddr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* pElement = a[a_addr];
    pElement->RemoveChild(a_childAddr);

    UIElement* cElement = a[a_childAddr];
    cElement->SetParent(-1);
}
uint32_t* UIControlBindings::GetElementChildren(uint32_t a_addr, uint32_t* a_count) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];

    *a_count = element->GetChildCount();
    return element->GetChildren();
}
glm::vec2 UIControlBindings::GetElementPosition(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];
    return element->GetPosition();
}
void UIControlBindings::SetElementPosition(uint32_t a_addr, const glm::vec2& a_pos) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* element = a[a_addr];
    element->SetPosition(a_pos);
}
glm::vec2 UIControlBindings::GetElementSize(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];
    return element->GetSize();
}
void UIControlBindings::SetElementSize(uint32_t a_addr, const glm::vec2& a_size) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* element = a[a_addr];
    element->SetSize(a_size);
}
glm::vec4 UIControlBindings::GetElementColor(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];
    return element->GetColor();
}
void UIControlBindings::SetElementColor(uint32_t a_addr, const glm::vec4& a_color) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* element = a[a_addr];
    element->SetColor(a_color);
}
e_UIXAnchor UIControlBindings::GetElementXAnchor(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];
    return element->GetXAnchor();
}
void UIControlBindings::SetElementXAnchor(uint32_t a_addr, e_UIXAnchor a_anchor) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* element = a[a_addr];
    element->SetXAnchor(a_anchor);
}
e_UIYAnchor UIControlBindings::GetElementYAnchor(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];
    return element->GetYAnchor();
}
void UIControlBindings::SetElementYAnchor(uint32_t a_addr, e_UIYAnchor a_anchor) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    UIElement* element = a[a_addr];
    element->SetYAnchor(a_anchor);
}
e_ElementState UIControlBindings::GetElementState(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const UIElement* element = a[a_addr];
    return element->GetState();
}

uint32_t UIControlBindings::CreateTextElement() const
{
    TRACE("Creating Text UI Element");
    TextUIElement* element = MallocAllocator::Instance->Create<TextUIElement>(MallocAllocator::Instance);

    return m_uiControl->m_uiElements.PushVal(element);
}
COWU32String UIControlBindings::GetTextElementText(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text);

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    TextUIElement* element = (TextUIElement*)a[a_addr];
    return element->GetText();
}
void UIControlBindings::SetTextElementText(uint32_t a_addr, const CharU32* a_text) const
{
    const COWU32String str = COWU32String(a_text, MallocAllocator::Instance);

    SetTextElementText(a_addr, str);
}
void UIControlBindings::SetTextElementText(uint32_t a_addr, const COWU32String& a_text) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text);

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    TextUIElement* element = (TextUIElement*)a[a_addr];
    element->SetText(a_text);
}
uint32_t UIControlBindings::GetTextElementFont(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text);

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const TextUIElement* element = (TextUIElement*)a[a_addr];
    return element->GetFontAddr();
}
void UIControlBindings::SetTextElementFont(uint32_t a_addr, uint32_t a_fontAddr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text);

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    TextUIElement* element = (TextUIElement*)a[a_addr];
    element->SetFontAddr(a_fontAddr);
}
float UIControlBindings::GetTextElementFontSize(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text);

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const TextUIElement* element = (TextUIElement*)a[a_addr];
    return element->GetFontSize();
}
void UIControlBindings::SetTextElementFontSize(uint32_t a_addr, float a_size) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Text);

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    TextUIElement* element = (TextUIElement*)a[a_addr];
    element->SetFontSize(a_size);
}

uint32_t UIControlBindings::CreateImageElement() const
{
    TRACE("Creating Image UI Element");
    ImageUIElement* element = MallocAllocator::Instance->Create<ImageUIElement>(MallocAllocator::Instance);

    return m_uiControl->m_uiElements.PushVal(element);
}
uint32_t UIControlBindings::GetImageElementSampler(uint32_t a_addr) const
{
    IVERIFY(m_uiControl->m_uiElements.Exists(a_addr));
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Image);

    const TReadLockArray<UIElement*> a = m_uiControl->m_uiElements.ToReadLockArray();

    const ImageUIElement* element = (ImageUIElement*)a[a_addr];
    return element->GetSamplerAddr();
}
void UIControlBindings::SetImageElementSampler(uint32_t a_addr, uint32_t a_samplerAddr) const
{
    IVERIFY(m_uiControl->m_uiElements[a_addr] != nullptr);
    IVERIFY(m_uiControl->m_uiElements[a_addr]->GetType() == UIElementType_Image);

    TLockArray<UIElement*> a = m_uiControl->m_uiElements.ToLockArray();

    ImageUIElement* element = (ImageUIElement*)a[a_addr];
    element->SetSamplerAddr(a_samplerAddr);
}

// MIT License
// 
// Copyright (c) 2026 River Govers
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