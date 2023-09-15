#pragma once

#include "Rendering/RenderEngineBackend.h"

class RuntimeManager;

class NullRenderEngineBackend : public RenderEngineBackend
{
private:

protected:

public:
    NullRenderEngineBackend(RuntimeManager* a_runtime, RenderEngine* a_engine);
    virtual ~NullRenderEngineBackend();

    virtual uint32_t GenerateAlphaTexture(uint32_t a_width, uint32_t a_height, const void* a_data);
    virtual void DestroyTexture(uint32_t a_texture);

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, FlareBase::e_TextureMode a_textureMode, FlareBase::e_TextureFilter a_filterMode, FlareBase::e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    virtual void DestroyTextureSampler(uint32_t a_sampler);

    virtual Font* GetFont(uint32_t a_addr);

    virtual void Update(double a_delta, double a_time);
};