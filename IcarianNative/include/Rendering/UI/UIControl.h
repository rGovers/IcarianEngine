#pragma once

#include <cstdint>
#include <vector>

#include "CanvasBuffer.h"
#include "DataTypes/TArray.h"

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

    UIControl(RuntimeManager* a_runtime);
    
protected:

public:
    ~UIControl();

    static void Init(RuntimeManager* a_runtime);
    static void Destroy();

    static std::vector<CanvasBuffer> GetCanvases();

    static CanvasBuffer GetCanvas(uint32_t a_addr);
    static void SetCanvas(uint32_t a_addr, const CanvasBuffer& a_buffer);

    static UIElement* GetUIElement(uint32_t a_addr);
};