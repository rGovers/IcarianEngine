#include "AssetLibrary.h"

#include "Flare/IcarianAssert.h"
#include "Rendering/UI/Font.h"
#include "Runtime/RuntimeManager.h"

static AssetLibrary* Instance = nullptr;

#define ASSETLIBRARY_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

#define ASSETLIBRARY_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering.UI, Font, GenerateFont, { char* str = mono_string_to_utf8(a_path); const uint32_t ret = AssetLibrary::GenerateFont(str); mono_free(str); return ret; }, MonoString* a_path) \
    F(void, IcarianEngine.Rendering.UI, Font, DestroyFont, { AssetLibrary::DestroyFont(a_addr); }, uint32_t a_addr)

ASSETLIBRARY_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

void AssetLibrary::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        Instance = new AssetLibrary();

        ASSETLIBRARY_BINDING_FUNCTION_TABLE(ASSETLIBRARY_RUNTIME_ATTACH);
    }
}
void AssetLibrary::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

AssetLibrary::AssetLibrary()
{

}
AssetLibrary::~AssetLibrary()
{
    for (uint32_t i = 0; i < m_fonts.Size(); ++i)
    {
        if (m_fonts[i] != nullptr)
        {
            Logger::Warning("Font not destroyed");

            delete m_fonts[i];
        }
    }
}

uint32_t AssetLibrary::GenerateFont(const std::string_view& a_path)
{
    ICARIAN_ASSERT_MSG(!a_path.empty(), "AddFont path empty");

    Font* font = new Font(a_path);

    uint32_t size = 0;
    {
        TLockArray<Font*> a = Instance->m_fonts.ToLockArray();
        
        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = font;

                return i;
            }
        }
    }

    Instance->m_fonts.Push(font);

    return size;
}
Font* AssetLibrary::GetFont(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_fonts.Size(), "GetFont out of bounds");

    return Instance->m_fonts[a_addr];
}
void AssetLibrary::DestroyFont(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_fonts.Size(), "DestroyFont out of bounds");

    Font* font = Instance->m_fonts[a_addr];
    Instance->m_fonts[a_addr] = nullptr;

    delete font;
}