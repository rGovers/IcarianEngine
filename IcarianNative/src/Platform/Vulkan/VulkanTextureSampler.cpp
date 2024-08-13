// Icarian Engine - C# Game Engine
// 
// License at end of file.

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanTextureSampler.h"

#include "Core/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanTexture.h"

class VulkanTextureSamplerDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Sampler                m_sampler;

protected:

public:
    VulkanTextureSamplerDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Sampler a_sampler)
    {
        m_engine = a_engine;

        m_sampler = a_sampler;
    }
    virtual ~VulkanTextureSamplerDeletionObject()
    {
        
    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroySampler(m_sampler);
    }
};

constexpr static vk::Filter GetFilterMode(e_TextureFilter a_filter)
{
    switch (a_filter)
    {
    case TextureFilter_Linear:
    {
        return vk::Filter::eLinear;
    }
    case TextureFilter_Nearest:
    {
        return vk::Filter::eNearest;
    }
    }

    ICARIAN_ASSERT(0);

    return vk::Filter::eNearest;
} 

constexpr static vk::SamplerAddressMode GetAddressMode(e_TextureAddress a_address)
{
    switch (a_address)
    {
    case TextureAddress_MirroredRepeat:
    {
        return vk::SamplerAddressMode::eMirroredRepeat;
    }
    case TextureAddress_ClampToEdge:
    {
        return vk::SamplerAddressMode::eClampToEdge;
    }
    case TextureAddress_Repeat:
    {
        return vk::SamplerAddressMode::eRepeat;
    }
    }

    ICARIAN_ASSERT(0);

    return vk::SamplerAddressMode::eRepeat;
}

VulkanTextureSampler::VulkanTextureSampler(VulkanRenderEngineBackend* a_engine)
{
    m_engine = a_engine;
}
VulkanTextureSampler::~VulkanTextureSampler()
{
    m_engine->PushDeletionObject(new VulkanTextureSamplerDeletionObject(m_engine, m_sampler));
}

VulkanTextureSampler* VulkanTextureSampler::GenerateFromBuffer(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const TextureSamplerBuffer& a_sampler)
{
    VulkanTextureSampler* sampler = new VulkanTextureSampler(a_engine);

    const vk::Device device = a_engine->GetLogicalDevice();

    const vk::Filter filter = GetFilterMode(a_sampler.FilterMode);
    const vk::SamplerAddressMode address = GetAddressMode(a_sampler.AddressMode);

    const vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo
    (
        { },
        filter,
        filter,
        vk::SamplerMipmapMode::eNearest,
        address,
        address,
        address
    );

    ICARIAN_ASSERT_MSG_R(device.createSampler(&samplerInfo, nullptr, &sampler->m_sampler) == vk::Result::eSuccess, "Failed to create texture sampler");

    return sampler;
}

#endif

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.