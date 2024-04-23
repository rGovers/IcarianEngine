#pragma once

#include <string>
#include <string_view>

#include "Rendering/RenderEngine.h"

class Config
{
private:
    static constexpr char DefaultAppName[] = "IcarianEngine";

    bool              m_headless = false;

    double            m_fixedTimeStep = 1.0 / 50.0;
    uint32_t          m_fileCacheSize = 256;

    std::string       m_appName = std::string(DefaultAppName);

    e_RenderingEngine m_renderingEngine = RenderingEngine_Vulkan;

protected:

public:
    Config(const std::string_view& a_path);
    ~Config();

    inline double GetFixedTimeStep() const
    {
        return m_fixedTimeStep;
    }

    inline uint32_t GetFileCacheSize() const
    {
        return m_fileCacheSize;
    }

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