#pragma once

#include <string>
#include <string_view>

#include "Rendering/RenderEngine.h"

class Config
{
private:
    static constexpr std::string_view DefaultAppName = "IcarianEngine";

    bool              m_headless = false;

    std::string       m_appName = std::string(DefaultAppName);

    e_RenderingEngine m_renderingEngine = RenderingEngine_Vulkan;

protected:

public:
    Config(const std::string_view& a_path);
    ~Config();

    inline const std::string_view GetApplicationName() const
    {
        return m_appName;
    }
    inline e_RenderingEngine GetRenderingEngine() const
    {
        return m_renderingEngine;
    }
    inline bool IsHeadless() const
    {
        return m_headless;
    }
    inline void SetHeadless(bool a_value)
    {
        m_headless = a_value;
    }
};