// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>

#include "DataTypes/TNCArray.h"

class RuntimeFunction;
class UIControlBindings;
class UIElement;

#include "EngineCanvasInteropStructures.h"

class UIControl
{
private:
    friend class UIControlBindings;

    static UIControl* Instance;

    UIControlBindings*     m_bindings;

    RuntimeFunction*       m_onNormal;
    RuntimeFunction*       m_onHover;
    RuntimeFunction*       m_onPressed;
    RuntimeFunction*       m_onReleased;

    TNCArray<CanvasBuffer> m_canvas;
    TNCArray<UIElement*>   m_uiElements;

    UIControl();
    
    void SendCursor(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_pos, const glm::vec2& a_screenSize);
    bool SendClick(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_pos, const glm::vec2& a_screenSize);
    void SendRelease(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_pos, const glm::vec2& a_screenSize);

protected:

public:
    ~UIControl();

    static void Init();
    static void Destroy();

    static CanvasBuffer GetCanvas(uint32_t a_addr);
    static void SetCanvas(uint32_t a_addr, const CanvasBuffer& a_buffer);

    static UIElement* GetUIElement(uint32_t a_addr);

    static void UpdateCursor(const glm::vec2& a_pos, const glm::vec2& a_size);
    static bool SubmitClick(const glm::vec2& a_pos, const glm::vec2& a_size);
    static void SubmitRelease(const glm::vec2& a_pos, const glm::vec2& a_size);
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