#pragma once

#include <cstdint>
#include <vector>

#include "CanvasBuffer.h"
#include "DataTypes/TArray.h"

class RuntimeFunction;
class RuntimeManager;
class UIControlBindings;
class UIElement;

class UIControl
{
private:
    friend class UIControlBindings;

    static UIControl* Instance;

    TArray<CanvasBuffer> m_canvas;
    TArray<UIElement*>   m_uiElements;

    UIControlBindings*   m_bindings;

    RuntimeFunction*     m_onNormal;
    RuntimeFunction*     m_onHover;
    RuntimeFunction*     m_onPressed;
    RuntimeFunction*     m_onReleased;

    UIControl(RuntimeManager* a_runtime);
    
    void SendCursor(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_scaledPos);
    bool SendClick(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_scaledPos);
    void SendRelease(uint32_t a_canvasAddr, uint32_t a_elementAddr, const glm::vec2& a_scaledPos);

protected:

public:
    ~UIControl();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    // TODO: Need to remove down the line when implementing canvas render system
    static std::vector<CanvasBuffer> GetCanvases();

    static CanvasBuffer GetCanvas(uint32_t a_addr);
    static void SetCanvas(uint32_t a_addr, const CanvasBuffer& a_buffer);

    static UIElement* GetUIElement(uint32_t a_addr);

    static void UpdateCursor(const glm::vec2& a_pos, const glm::vec2& a_size);
    static bool SubmitClick(const glm::vec2& a_pos, const glm::vec2& a_size);
    static void SubmitRelease(const glm::vec2& a_pos, const glm::vec2& a_size);
};