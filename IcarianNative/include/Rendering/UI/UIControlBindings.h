#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <string>

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
    uint32_t* GetCanvasChildren(uint32_t a_addr, uint32_t* a_count) const;

    void AddElementChild(uint32_t a_addr, uint32_t a_childAddr) const;
    void RemoveElementChild(uint32_t a_addr, uint32_t a_childAddr) const;
    uint32_t* GetElementChildren(uint32_t a_addr, uint32_t* a_count) const;
    glm::vec2 GetElementPosition(uint32_t a_addr) const;
    void SetElementPosition(uint32_t a_addr, const glm::vec2& a_pos) const;
    glm::vec2 GetElementSize(uint32_t a_addr) const;
    void SetElementSize(uint32_t a_addr, const glm::vec2& a_size) const;
    glm::vec4 GetElementColor(uint32_t a_addr) const;
    void SetElementColor(uint32_t a_addr, const glm::vec4& a_color) const;
    uint32_t GetElementState(uint32_t a_addr) const;

    uint32_t CreateTextElement() const;
    void DestroyTextElement(uint32_t a_addr) const;
    std::u32string GetTextElementText(uint32_t a_addr) const;
    void SetTextElementText(uint32_t a_addr, const std::u32string_view& a_text) const;
    uint32_t GetTextElementFont(uint32_t a_addr) const;
    void SetTextElementFont(uint32_t a_addr, uint32_t a_fontAddr) const;
    float GetTextElementFontSize(uint32_t a_addr) const;
    void SetTextElementFontSize(uint32_t a_addr, float a_size) const;

    uint32_t CreateImageElement() const;
    void DestroyImageElement(uint32_t a_addr) const;
    uint32_t GetImageElementSampler(uint32_t a_addr) const;
    void SetImageElementSampler(uint32_t a_addr, uint32_t a_samplerAddr) const;
};