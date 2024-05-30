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