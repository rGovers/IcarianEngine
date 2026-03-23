// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/RenderAssetStoreBindings.h"

#include "Core/IcarianDefer.h"
#include "DataTypes/Allocators/MallocAllocator.h"
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

    Array<Vertex> vertices = Array<Vertex>(MallocAllocator::Instance);
    Array<uint32_t> indices = Array<uint32_t>(MallocAllocator::Instance);
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

uint32_t RenderAssetStoreBindings::GenerateFont(const std::string_view& a_path) const
{
    Font* font = Font::LoadFont(a_path);
    IVERIFY(font != nullptr);

    return m_store->m_data->Fonts.PushVal(font);
}
void RenderAssetStoreBindings::DestroyFont(uint32_t a_addr) const
{
    IVERIFY(m_store->m_data->Fonts.Exists(a_addr));

    const Font* font = m_store->m_data->Fonts[a_addr];
    IDEFER(delete font);
    m_store->m_data->Fonts.Erase(a_addr);
}
uint32_t RenderAssetStoreBindings::GenerateModelFromString(uint32_t a_addr, const std::u32string_view& a_str, float a_fontSize, float a_scale, float a_depth) const
{
    IVERIFY(m_store->m_data->Fonts.Exists(a_addr));

    const Font* font = m_store->m_data->Fonts[a_addr];

    Array<Vertex> vertices = Array<Vertex>(m_store->m_blockAllocator);
    Array<uint32_t> indices = Array<uint32_t>(m_store->m_blockAllocator);
    float radius;
    font->StringToModel(a_str, a_fontSize, a_scale, a_depth, &vertices, &indices, &radius);

    if (radius > 0 && !vertices.Empty() && !indices.Empty())
    {
        return m_store->m_data->Renderer->GenerateModel
        (
            vertices.Data(),
            vertices.Size(),
            sizeof(Vertex),
            indices.Data(),
            indices.Size(),
            radius
        );
    }

    return -1;
}

bool RenderAssetStoreBindings::LoadModelData(const std::string_view& a_path, uint32_t a_index, Array<Vertex>* a_vertices, Array<uint32_t>* a_indices) const
{
    float rad;

    return m_store->LoadModelData(a_path, (uint8_t)a_index, a_vertices, a_indices, &rad);
}

uint32_t RenderAssetStoreBindings::GenerateMesh(const std::string_view& a_path, uint32_t a_index) const
{
    return m_store->LoadMesh(a_path, (uint8_t)a_index);
}

uint32_t RenderAssetStoreBindings::GenerateModel(const std::string_view& a_path, uint32_t a_index) const
{
    return m_store->LoadModel(a_path, (uint8_t)a_index);
}
uint32_t RenderAssetStoreBindings::GenerateSkinnedModel(const std::string_view& a_path, uint32_t a_index) const
{
    return m_store->LoadSkinnedModel(a_path, a_index);
}

uint32_t RenderAssetStoreBindings::GenerateTexture(const std::string_view& a_path) const
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
