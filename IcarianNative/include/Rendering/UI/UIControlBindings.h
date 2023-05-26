#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

class RuntimeManager;
class UIControl;

class UIControlBindings
{
private:
    UIControl* m_uiControl;

protected:

public:
    UIControlBindings(UIControl* a_uiControl, RuntimeManager* a_runtime);
    ~UIControlBindings();

    uint32_t CreateCanvas(const glm::vec2& a_refResolution) const;
    void DestroyCanvas(uint32_t a_addr) const;
    void AddCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const;
    void RemoveCanvasChild(uint32_t a_addr, uint32_t a_uiElementAddr) const;

    uint32_t CreateTextElement() const;
    void DestroyTextElement(uint32_t a_addr) const;
};