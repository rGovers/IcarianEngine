// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include <cstdint>
#include <string>
#include <unordered_map>

#include "Rendering/Vulkan/IcarianVulkanHeader.h"

class Allocator;
class VulkanComputeShader;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanVertexShader;

#include "EngineMaterialInteropStructures.h"

struct VulkanMeshFShaderBuilder
{
    VulkanRenderEngineBackend* Engine;
    VulkanGraphicsEngine* GraphicsEngine;
    std::string String;
    std::unordered_map<std::string, std::string> Imports;
    std::string EntryPoint;
};

class VulkanMeshShader
{
private:
    // Removed GLSL generation and no longer inherits from Shader as it can be multiple shaders if emulating
    VulkanRenderEngineBackend*  m_engine;
    VulkanGraphicsEngine*       m_gEngine;
    Allocator*                  m_allocator;

    // Yes code smell but will not use both at once
    // Memory safety people are probably screaming murder
    union MeshUnion
    {
        struct
        {
            vk::ShaderModule Module;

            ShaderBufferInput* Inputs;
            uint32_t InputCount;
        } Native;
        struct
        {
            VulkanVertexShader* VertexShader;
            VulkanComputeShader* ComputeShader;
        } Emulated;

        MeshUnion()
        {
            Emulated.VertexShader = nullptr;
            Emulated.ComputeShader = nullptr;
        }
    } m_data;

    VulkanMeshShader(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanVertexShader* a_vertexShader, VulkanComputeShader* a_computeShader, Allocator* a_allocator);
    VulkanMeshShader(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const ShaderBufferInput* a_inputs, uint32_t a_inputCount, vk::ShaderModule a_module, Allocator* a_allocator);

protected:

public:
    VulkanMeshShader() = delete;
    virtual ~VulkanMeshShader();

    uint32_t GetShaderInputCount() const;
    ShaderBufferInput GetShaderInput(uint32_t a_index) const;

    vk::ShaderModule GetShaderModule() const;

    VulkanVertexShader* GetVertexShader() const;
    VulkanComputeShader* GetComputeShader() const;

    static void CreateFromFShader(VulkanMeshShader* a_out, const VulkanMeshFShaderBuilder& a_builder, Allocator* a_allocator);
};

#endif

// MIT License
// 
// Copyright (c) 2025 River Govers
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
