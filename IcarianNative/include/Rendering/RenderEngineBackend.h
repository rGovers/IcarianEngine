#pragma once

#include <cstdint>

#include "Rendering/TextureData.h"

class Font;
class RenderEngine;

#include "EngineTextureSamplerInteropStructures.h"

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

    virtual uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, uint16_t a_vertexStride, const uint32_t* a_indices, uint32_t a_indexCount, float a_radius) = 0;
    virtual void DestroyModel(uint32_t a_addr) = 0;

    virtual uint32_t GenerateTexture(uint32_t a_width, uint32_t a_height, e_TextureFormat a_format, const void* a_data) = 0;
    virtual uint32_t GenerateTextureMipMapped(uint32_t a_width, uint32_t a_height, uint32_t a_levels, uint64_t* a_offsets, e_TextureFormat a_format, const void* a_data, uint64_t a_dataSize) = 0;
    virtual void DestroyTexture(uint32_t a_addr) = 0;

    virtual uint32_t GenerateTextureSampler(uint32_t a_textureAddr, e_TextureMode a_textureMode, e_TextureFilter a_filterMode, e_TextureAddress a_addressMode, uint32_t a_slot = 0) = 0;
    virtual void DestroyTextureSampler(uint32_t a_addr) = 0;

    virtual void Update(double a_delta, double a_time) = 0;
};