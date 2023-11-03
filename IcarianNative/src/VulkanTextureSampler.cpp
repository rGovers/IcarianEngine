#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include "Rendering/Vulkan/VulkanTextureSampler.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Trace.h"

class VulkanTextureSamplerDeletionObject : public VulkanDeletionObject
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::Sampler                m_sampler;
    vk::ImageView              m_view;

protected:

public:
    VulkanTextureSamplerDeletionObject(VulkanRenderEngineBackend* a_engine, vk::Sampler a_sampler, vk::ImageView a_view)
    {
        m_engine = a_engine;

        m_sampler = a_sampler;
        m_view = a_view;
    }
    virtual ~VulkanTextureSamplerDeletionObject()
    {
        
    }

    virtual void Destroy()
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        device.destroySampler(m_sampler);
        if (m_view != vk::ImageView(nullptr))
        {
            device.destroyImageView(m_view);
        }
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
    }

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
    }

    return vk::SamplerAddressMode::eRepeat;
}

VulkanTextureSampler::VulkanTextureSampler(VulkanRenderEngineBackend* a_engine)
{
    m_engine = a_engine;
}
VulkanTextureSampler::~VulkanTextureSampler()
{
    m_engine->PushDeletionObject(new VulkanTextureSamplerDeletionObject(m_engine, m_sampler, m_view));
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

    switch (a_sampler.TextureMode)
    {
    case TextureMode_Texture:
    {
        const VulkanTexture* texture = a_gEngine->GetTexture(a_sampler.Addr);
        ICARIAN_ASSERT(texture != nullptr);

        constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        const vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo
        (
            { }, 
            texture->GetImage(), 
            vk::ImageViewType::e2D, 
            texture->GetFormat(),
            { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
            SubresourceRange
        );

        ICARIAN_ASSERT_MSG_R(device.createImageView(&viewInfo, nullptr, &sampler->m_view) == vk::Result::eSuccess, "Failed to create image view");

        break;
    }
    }

    return sampler;
}
#endif