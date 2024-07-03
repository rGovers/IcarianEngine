#include "Rendering/RenderAssetStoreBindings.h"

#include "Core/IcarianDefer.h"
#include "DeletionQueue.h"
#include "IcarianError.h"
#include "Rendering/RenderAssetStore.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/Font.h"
#include "Runtime/RuntimeManager.h"

#include "EngineFontInterop.h"

static RenderAssetStoreBindings* Instance = nullptr;

#define RENDERASSETSTORE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateModel(str, a_index)); }, MonoString* a_path, uint32_t a_index) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateSkinnedFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateSkinnedModel(str, a_index)); }, MonoString* a_path, uint32_t a_index) \
    \
    F(uint32_t, IcarianEngine.Rendering, Texture, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateTexture(str)); }, MonoString* a_path) \

RENDERASSETSTORE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINE_FONT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

RenderAssetStoreBindings::RenderAssetStoreBindings(RenderAssetStore* a_store)
{
    Instance = this;

    m_store = a_store;

    RENDERASSETSTORE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINE_FONT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
RenderAssetStoreBindings::~RenderAssetStoreBindings()
{

}

uint32_t RenderAssetStoreBindings::GenerateFont(const std::filesystem::path& a_path) const
{
    Font* font = Font::LoadFont(a_path);
    IVERIFY(font != nullptr);

    return m_store->m_fonts.PushVal(font);
}
void RenderAssetStoreBindings::DestroyFont(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_store->m_fonts.Size());
    IVERIFY(m_store->m_fonts.Exists(a_addr));

    const Font* font = m_store->m_fonts[a_addr];
    IDEFER(delete font);
    m_store->m_fonts.Erase(a_addr);
}
uint32_t RenderAssetStoreBindings::GenerateModelFromString(uint32_t a_addr, const std::u32string_view& a_str, float a_fontSize, float a_scale, float a_depth) const
{
    IVERIFY(a_addr < m_store->m_fonts.Size());
    IVERIFY(m_store->m_fonts.Exists(a_addr));

    const Font* font = m_store->m_fonts[a_addr];

    Array<Vertex> vertices;
    Array<uint32_t> indices;
    float radius;
    font->StringToModel(a_str, a_fontSize, a_scale, a_depth, &vertices, &indices, &radius);
    
    if (radius > 0 && !vertices.Empty() && !indices.Empty())
    {
        return m_store->m_renderEngine->GenerateModel(vertices.Data(), vertices.Size(), sizeof(Vertex), indices.Data(), indices.Size(), radius);
    }

    return -1;
}

uint32_t RenderAssetStoreBindings::GenerateModel(const std::filesystem::path& a_path, uint32_t a_index) const
{
    return m_store->LoadModel(a_path, a_index);
}
uint32_t RenderAssetStoreBindings::GenerateSkinnedModel(const std::filesystem::path& a_path, uint32_t a_index) const
{
    return m_store->LoadSkinnedModel(a_path, a_index);
}

uint32_t RenderAssetStoreBindings::GenerateTexture(const std::filesystem::path& a_path) const
{
    return m_store->LoadTexture(a_path);
}