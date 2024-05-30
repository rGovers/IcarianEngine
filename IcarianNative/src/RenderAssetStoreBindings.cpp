#include "Rendering/RenderAssetStoreBindings.h"

#include "Core/IcarianDefer.h"
#include "DeletionQueue.h"
#include "IcarianError.h"
#include "Rendering/RenderAssetStore.h"
#include "Rendering/UI/Font.h"
#include "Runtime/RuntimeManager.h"

#include "EngineFontInterop.h"

static RenderAssetStoreBindings* Instance = nullptr;

#define RENDERASSETSTORE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateModel(str)); }, MonoString* a_path) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateSkinnedFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateSkinnedModel(str)); }, MonoString* a_path) \
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

uint32_t RenderAssetStoreBindings::GenerateModel(const std::filesystem::path& a_path) const
{
    return m_store->LoadModel(a_path);
}
uint32_t RenderAssetStoreBindings::GenerateSkinnedModel(const std::filesystem::path& a_path) const
{
    return m_store->LoadSkinnedModel(a_path);
}

uint32_t RenderAssetStoreBindings::GenerateTexture(const std::filesystem::path& a_path) const
{
    return m_store->LoadTexture(a_path);
}