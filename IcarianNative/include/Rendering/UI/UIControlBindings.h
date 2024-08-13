// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <string>

class RuntimeManager;
class UIControl;

#include "EngineUIElementInteropStuctures.h"

class UIControlBindings
{
private:
    UIControl* m_uiControl;

protected:

public:
    UIControlBindings(UIControl* a_uiControl);
    ~UIControlBindings();

    uint32_t CreateCanvas(const glm::vec2& a_refResolution) const;
    void DestroyCanvas(uint32_t a_addr) const;
    void AddCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const;
    void RemoveCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const;
    uint32_t* GetCanvasChildren(uint32_t a_addr, uint32_t* a_count) const;

    uint32_t CreateUIElement() const;
    void DestroyUIElement(uint32_t a_addr) const;
    void AddElementChild(uint32_t a_addr, uint32_t a_childAddr) const;
    void RemoveElementChild(uint32_t a_addr, uint32_t a_childAddr) const;
    uint32_t* GetElementChildren(uint32_t a_addr, uint32_t* a_count) const;
    glm::vec2 GetElementPosition(uint32_t a_addr) const;
    void SetElementPosition(uint32_t a_addr, const glm::vec2& a_pos) const;
    glm::vec2 GetElementSize(uint32_t a_addr) const;
    void SetElementSize(uint32_t a_addr, const glm::vec2& a_size) const;
    glm::vec4 GetElementColor(uint32_t a_addr) const;
    void SetElementColor(uint32_t a_addr, const glm::vec4& a_color) const;
    e_UIXAnchor GetElementXAnchor(uint32_t a_addr) const;
    void SetElementXAnchor(uint32_t a_addr, e_UIXAnchor a_anchor) const;
    e_UIYAnchor GetElementYAnchor(uint32_t a_addr) const;
    void SetElementYAnchor(uint32_t a_addr, e_UIYAnchor a_anchor) const;
    e_ElementState GetElementState(uint32_t a_addr) const;

    uint32_t CreateTextElement() const;
    std::u32string GetTextElementText(uint32_t a_addr) const;
    void SetTextElementText(uint32_t a_addr, const std::u32string_view& a_text) const;
    uint32_t GetTextElementFont(uint32_t a_addr) const;
    void SetTextElementFont(uint32_t a_addr, uint32_t a_fontAddr) const;
    float GetTextElementFontSize(uint32_t a_addr) const;
    void SetTextElementFontSize(uint32_t a_addr, float a_size) const;

    uint32_t CreateImageElement() const;
    uint32_t GetImageElementSampler(uint32_t a_addr) const;
    void SetImageElementSampler(uint32_t a_addr, uint32_t a_samplerAddr) const;
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