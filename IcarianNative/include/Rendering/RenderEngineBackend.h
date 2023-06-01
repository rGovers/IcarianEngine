#pragma once

#include <cstdint>
#include <string_view>

#include "Flare/TextureSampler.h"

class Font;
class RenderEngine;

class RenderEngineBackend
{
private:
    RenderEngine* m_renderEngine;
    
protected:

public:
    RenderEngineBackend(RenderEngine* a_engine) 
    {
        m_renderEngine = a_engine;
    }
    virtual ~RenderEngineBackend() { }

    inline RenderEngine* GetRenderEngine() const
    {
        return m_renderEngine;
    }

    virtual uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data) = 0;
    virtual void DestroyTexture(uint32_t a_addr) = 0;

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot = 0) = 0;
    virtual void DestroyTextureSampler(uint32_t a_addr) = 0;

    virtual Font* GetFont(uint32_t a_addr) = 0;

    virtual void Update(double a_delta, double a_time) = 0;
};