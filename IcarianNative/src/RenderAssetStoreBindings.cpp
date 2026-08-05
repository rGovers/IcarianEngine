// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Rendering/RenderAssetStoreBindings.h"

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "DeletionQueue.h"
#include "IcarianError.h"
#include "Rendering/RenderAssetStore.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/UI/Font.h"
#include "Runtime/RuntimeManager.h"

#include "EngineFontInterop.h"

static RenderAssetStoreBindings* Instance = nullptr;

#define RENDERASSETSTORE_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, IcarianEngine.Rendering, Mesh, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateMesh(str, a_index)); }, MonoString* a_path, uint32_t a_index) \
    \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateModel(str, a_index)); }, MonoString* a_path, uint32_t a_index) \
    F(uint32_t, IcarianEngine.Rendering, Model, GenerateSkinnedFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateSkinnedModel(str, a_index)); }, MonoString* a_path, uint32_t a_index) \
    \
    F(uint32_t, IcarianEngine.Rendering, Texture, GenerateFromFile, { char* str = mono_string_to_utf8(a_path); IDEFER(mono_free(str)); return TORENDERSTOREADDR(Instance->GenerateTexture(str)); }, MonoString* a_path) \

RENDERASSETSTORE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION);

ENGINE_FONT_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

RUNTIME_FUNCTION(ModelDataStructure, Model, GetModelData,
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    ModelDataStructure s = { 0 };

    IcarianCore::Array<Vertex> vertices = IcarianCore::Array<Vertex>(IcarianCore::MallocAllocator::Instance);
    IcarianCore::Array<uint32_t> indices = IcarianCore::Array<uint32_t>(IcarianCore::MallocAllocator::Instance);
    if (Instance->LoadModelData(str, a_index, &vertices, &indices))
    {
        MonoDomain* domain = mono_domain_get();

        MonoClass* uint32Class = mono_get_uint32_class();
        MonoClass* vertexClass = RuntimeManager::GetClass("IcarianEngine.Rendering", "Vertex");

        const uint32_t vertexCount = vertices.Size();
        s.Vertices = mono_array_new(domain, vertexClass, (uintptr_t)vertexCount);
        for (uint32_t i = 0; i < vertexCount; ++i)
        {
            mono_array_set(s.Vertices, Vertex, i, vertices[i]);
        }

        const uint32_t indexCount = indices.Size();
        s.Indices = mono_array_new(domain, uint32Class, (uintptr_t)indexCount);
        for (uint32_t i = 0; i < indexCount; ++i)
        {
            mono_array_set(s.Indices, uint32_t, i, indices[i]);
        }
    }

    return s;
}, MonoString* a_path, uint32_t a_index)

RenderAssetStoreBindings::RenderAssetStoreBindings(RenderAssetStore* a_store)
{
    Instance = this;

    m_store = a_store;

    RENDERASSETSTORE_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);

    ENGINE_FONT_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);

    BIND_FUNCTION(IcarianEngine.Rendering, Model, GetModelData);
}
RenderAssetStoreBindings::~RenderAssetStoreBindings()
{

}

uint32_t RenderAssetStoreBindings::GenerateFont(const char* a_path) const
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, m_store->m_blockAllocator);

    return GenerateFont(str);
}
uint32_t RenderAssetStoreBindings::GenerateFont(const IcarianCore::COWU8String& a_path) const
{
    IERRBLOCK;

    Font* font = m_store->m_blockAllocator->ZTAllocate<Font>();
    IERRDEFER(m_store->m_blockAllocator->Free(font));
    IERRCHECKRET(Font::LoadFont(font, a_path, m_store->m_blockAllocator), uint32_t(-1));

    return m_store->m_data->Fonts.PushVal(font);
}
void RenderAssetStoreBindings::DestroyFont(uint32_t a_addr) const
{
    IVERIFY(m_store->m_data->Fonts.Exists(a_addr));

    Font* font = m_store->m_data->Fonts[a_addr];
    IDEFER(m_store->m_blockAllocator->Destroy(font));

    m_store->m_data->Fonts.Erase(a_addr);
}

uint32_t RenderAssetStoreBindings::GenerateModelFromString(uint32_t a_addr, const IcarianCore::CharU32* a_str, float a_fontSize, float a_scale, float a_depth) const
{
    const IcarianCore::COWU32String str = IcarianCore::COWU32String(a_str, m_store->m_blockAllocator);

    return GenerateModelFromString(a_addr, str, a_fontSize, a_scale, a_depth);
}
uint32_t RenderAssetStoreBindings::GenerateModelFromString
(
    uint32_t a_addr,
    const IcarianCore::COWU32String& a_str,
    float a_fontSize,
    float a_scale,
    float a_depth
) const
{
    IVERIFY(m_store->m_data->Fonts.Exists(a_addr));

    const Font* font = m_store->m_data->Fonts[a_addr];

    IcarianCore::Array<Vertex> vertices = IcarianCore::Array<Vertex>(m_store->m_blockAllocator);
    IcarianCore::Array<uint32_t> indices = IcarianCore::Array<uint32_t>(m_store->m_blockAllocator);
    float radius;
    font->StringToModel(a_str, a_fontSize, a_scale, a_depth, &vertices, &indices, &radius, m_store->m_blockAllocator, m_store->m_blockAllocator);

    if (radius <= 0)
    {
        return uint32_t(-1);
    }

    const uint32_t vertexCount = vertices.Size();
    if (vertexCount <= 0)
    {
        return uint32_t(-1);
    }

    const uint32_t indexCount = indices.Size();
    if (indexCount <= 0)
    {
        return uint32_t(-1);
    }

    return m_store->m_data->Renderer->GenerateModel
    (
        vertices.Data(),
        vertexCount,
        sizeof(Vertex),
        indices.Data(),
        indexCount,
        radius
    );
}

bool RenderAssetStoreBindings::LoadModelData
(
    const char* a_path,
    uint32_t a_index,
    IcarianCore::Array<Vertex>* a_vertices,
    IcarianCore::Array<uint32_t>* a_indices
) const
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, m_store->m_blockAllocator);

    return LoadModelData(str, a_index, a_vertices, a_indices);
}
bool RenderAssetStoreBindings::LoadModelData
(
    const IcarianCore::COWU8String& a_path,
    uint32_t a_index,
    IcarianCore::Array<Vertex>* a_vertices,
    IcarianCore::Array<uint32_t>* a_indices
) const
{
    float rad;
    return m_store->LoadModelData(a_path, (uint8_t)a_index, a_vertices, a_indices, &rad);
}

uint32_t RenderAssetStoreBindings::GenerateMesh(const char* a_path, uint32_t a_index) const
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, m_store->m_blockAllocator);

    return GenerateMesh(str, a_index);
}
uint32_t RenderAssetStoreBindings::GenerateMesh(const IcarianCore::COWU8String& a_path, uint32_t a_index) const
{
    return m_store->LoadMesh(a_path, (uint8_t)a_index);
}

uint32_t RenderAssetStoreBindings::GenerateModel(const char* a_path, uint32_t a_index) const
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, m_store->m_blockAllocator);

    return GenerateModel(str, a_index);
}
uint32_t RenderAssetStoreBindings::GenerateModel(const IcarianCore::COWU8String& a_path, uint32_t a_index) const
{
    return m_store->LoadModel(a_path, (uint8_t)a_index);
}
uint32_t RenderAssetStoreBindings::GenerateSkinnedModel(const char* a_path, uint32_t a_index) const
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, m_store->m_blockAllocator);

    return GenerateSkinnedModel(str, a_index);
}
uint32_t RenderAssetStoreBindings::GenerateSkinnedModel(const IcarianCore::COWU8String& a_path, uint32_t a_index) const
{
    return m_store->LoadSkinnedModel(a_path, a_index);
}

uint32_t RenderAssetStoreBindings::GenerateTexture(const char* a_path) const
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, m_store->m_blockAllocator);

    return GenerateTexture(str);
}
uint32_t RenderAssetStoreBindings::GenerateTexture(const IcarianCore::COWU8String& a_path) const
{
    return m_store->LoadTexture(a_path);
}

// MIT License
//
// Copyright (c) 2026 River Govers
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
