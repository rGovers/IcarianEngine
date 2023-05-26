#include "Rendering/UI/UIControl.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/UI/UIControlBindings.h"
#include "Trace.h"

UIControl* UIControl::Instance = nullptr;

UIControl::UIControl(RuntimeManager* a_runtime)
{
    m_bindings = new UIControlBindings(this, a_runtime);
}
UIControl::~UIControl()
{
    delete m_bindings;
}

void UIControl::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new UIControl(a_runtime);
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