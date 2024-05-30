#pragma once

#include "Rendering/RenderEngineBackend.h"

class NullRenderEngineBackend : public RenderEngineBackend
{
private:

protected:

public:
    NullRenderEngineBackend(RenderEngine* a_engine);
    virtual ~NullRenderEngineBackend();

    virtual uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius);
    virtual void DestroyModel(uint32_t a_addr);

    virtual uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data);
    virtual uint32_t GenerateTextureMipMapped(uint32_t a_width, uint32_t a_height, uint32_t a_levels, uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize);
    virtual void DestroyTexture(uint32_t a_texture);

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0);
    virtual void DestroyTextureSampler(uint32_t a_sampler);

    virtual void Update(double a_delta, double a_time);
};