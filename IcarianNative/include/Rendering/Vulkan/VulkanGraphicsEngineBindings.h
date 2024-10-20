// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_VULKAN

#include <string_view>

class VulkanGraphicsEngine;
class VulkanPixelShader;
class VulkanVertexShader;

#include "Rendering/CameraBuffer.h"

#include "EngineAmbientLightInteropStructures.h"
#include "EngineDirectionalLightInteropStructures.h"
#include "EngineLightInteropStructures.h"
#include "EngineMaterialInteropStructures.h"
#include "EnginePointLightInteropStructures.h"
#include "EngineRenderCommandInteropStructures.h"
#include "EngineSpotLightInteropStructures.h"
#include "EngineTextureSamplerInteropStructures.h"

// Before someone tries to get rid of this compiler seems to be pretty good at optimizing this out so until shown otherwise I'm going to leave it
// Just glue code to get back into C++ style code from 
// Goes from C# -> Preprocessor written code -> C++
class VulkanGraphicsEngineBindings
{
private:
    VulkanGraphicsEngine* m_graphicsEngine;

protected:

public:
    VulkanGraphicsEngineBindings(VulkanGraphicsEngine* a_graphicsEngine);
    ~VulkanGraphicsEngineBindings();

    uint32_t GenerateFVertexShaderAddr(const std::string_view& a_str) const;
    void DestroyVertexShader(uint32_t a_addr) const;

    uint32_t GenerateFPixelShaderAddr(const std::string_view& a_str) const;
    void DestroyPixelShader(uint32_t a_addr) const;

    uint32_t GenerateShaderProgram(const RenderProgram& a_program) const;
    void DestroyShaderProgram(uint32_t a_addr) const;
    void RenderProgramSetTexture(uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) const;
    void RenderProgramSetUserUBO(uint32_t a_addr, uint32_t a_uboSize, const void* a_uboData) const;
    RenderProgram GetRenderProgram(uint32_t a_addr) const;
    void SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program) const;

    uint32_t GenerateCameraBuffer(uint32_t a_transformAddr) const;
    void DestroyCameraBuffer(uint32_t a_addr) const;
    CameraBuffer GetCameraBuffer(uint32_t a_addr) const;
    void SetCameraBuffer(uint32_t a_add, const CameraBuffer& a_buffer) const;
    glm::vec3 CameraScreenToWorld(uint32_t a_addr, const glm::vec3& a_screenPos, const glm::vec2& a_screenSize) const;
    glm::mat4 GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const;
    glm::mat4 GetCameraProjectionMatrix(uint32_t a_addr, uint32_t a_width, uint32_t a_height, float a_near, float a_far) const;

    uint32_t GenerateModel(const void* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride, float a_radius) const;
    void DestroyModel(uint32_t a_addr) const;

    uint32_t GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const;
    void DestroyMeshRenderBuffer(uint32_t a_addr) const;
    void GenerateRenderStack(uint32_t a_meshAddr) const;
    void DestroyRenderStack(uint32_t a_meshAddr) const;

    uint32_t GenerateSkinnedMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr, uint32_t a_skeletonAddr) const;
    void DestroySkinnedMeshRenderBuffer(uint32_t a_addr) const;
    void GenerateSkinnedRenderStack(uint32_t a_addr) const;
    void DestroySkinnedRenderStack(uint32_t a_addr) const;

    uint32_t GenerateGraphicsParticle2D(uint32_t a_computeBufferAddr) const;
    void DestroyGraphicsParticle2D(uint32_t a_addr) const;

    void DestroyTexture(uint32_t a_addr) const;

    uint32_t GenerateVideoTexture(uint32_t a_videoAddr) const;
    void DestroyVideoTexture(uint32_t a_addr) const;

    uint32_t GenerateTextureSampler(uint32_t a_texture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const;
    uint32_t GenerateRenderTextureSampler(uint32_t a_renderTexture, uint32_t a_textureIndex, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const;
    uint32_t GenerateRenderTextureDepthSampler(uint32_t a_renderTexture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const;
    uint32_t GenerateRenderTextureDepthSamplerDepth(uint32_t a_renderTexture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const;
    void DestroyTextureSampler(uint32_t a_addr) const;

    uint32_t GenerateRenderTexture(uint32_t a_count, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr, uint32_t a_channelCount) const;
    uint32_t GenerateRenderTextureD(uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthHandle, bool a_hdr, uint32_t a_channelCount) const;
    void DestroyRenderTexture(uint32_t a_addr) const;
    uint32_t GetRenderTextureTextureCount(uint32_t a_addr) const;
    bool RenderTextureHasDepth(uint32_t a_addr) const;
    uint32_t GetRenderTextureWidth(uint32_t a_addr) const;
    uint32_t GetRenderTextureHeight(uint32_t a_addr) const;
    void ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const;

    uint32_t GenerateDepthRenderTexture(uint32_t a_width, uint32_t a_height) const;
    void DestroyDepthRenderTexture(uint32_t a_addr) const;
    uint32_t GetDepthRenderTextureWidth(uint32_t a_addr) const;
    uint32_t GetDepthRenderTextureHeight(uint32_t a_addr) const;
    void ResizeDepthRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const;

    uint32_t GenerateDepthCubeRenderTexture(uint32_t a_width, uint32_t a_height) const;
    void DestroyDepthCubeRenderTexture(uint32_t a_addr) const;
    uint32_t GetDepthCubeRenderTextureWidth(uint32_t a_addr) const;
    uint32_t GetDepthCubeRenderTextureHeight(uint32_t a_addr) const;
    void ResizeDepthCubeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const;

    uint32_t GenerateAmbientLightBuffer() const;
    void SetAmbientLightBuffer(uint32_t a_addr, const AmbientLightBuffer& a_buffer) const;
    AmbientLightBuffer GetAmbientLightBuffer(uint32_t a_addr) const;
    void DestroyAmbientLightBuffer(uint32_t a_addr) const;

    uint32_t GenerateDirectionalLightBuffer(uint32_t a_transformAddr) const;
    void SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const;
    DirectionalLightBuffer GetDirectionalLightBuffer(uint32_t a_addr) const;
    void DestroyDirectionalLightBuffer(uint32_t a_addr) const;
    void AddDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const;
    void RemoveDirectionalLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const;

    uint32_t GeneratePointLightBuffer(uint32_t a_transformAddr) const;
    void SetPointLightBuffer(uint32_t a_addr, const PointLightBuffer& a_buffer) const;
    PointLightBuffer GetPointLightBuffer(uint32_t a_addr) const;
    void DestroyPointLightBuffer(uint32_t a_addr) const;
    void SetPointLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const;
    uint32_t GetPointLightShadowMap(uint32_t a_addr) const;

    uint32_t GenerateSpotLightBuffer(uint32_t a_transformAddr) const;
    void SetSpotLightBuffer(uint32_t a_addr, const SpotLightBuffer& a_buffer) const;
    SpotLightBuffer GetSpotLightBuffer(uint32_t a_addr) const;
    void DestroySpotLightBuffer(uint32_t a_addr) const;
    void SetSpotLightShadowMap(uint32_t a_addr, uint32_t a_shadowMapAddr) const;
    uint32_t GetSpotLightShadowMap(uint32_t a_addr) const;

    uint32_t GenerateFont(const std::string_view& a_path) const;
    void DestroyFont(uint32_t a_addr) const;

    uint32_t GenerateCanvasRenderer() const;
    void DestroyCanvasRenderer(uint32_t a_addr) const;
    void SetCanvasRendererCanvas(uint32_t a_addr, uint32_t a_canvasAddr) const;
    uint32_t GetCanvasRendererCanvas(uint32_t a_addr) const;
    void SetCanvasRendererRenderTexture(uint32_t a_addr, uint32_t a_renderTextureAddr) const;
    uint32_t GetCanvasRendererRenderTexture(uint32_t a_addr) const;

    void BindMaterial(uint32_t a_addr) const;
    void PushTexture(uint32_t a_slot, uint32_t a_samplerAddr) const;
    void PushLight(uint32_t a_slot, e_LightType a_lightType, uint32_t a_lightAddr) const;
    void PushLightSplits(uint32_t a_slot, const LightShadowSplit* a_splits, uint32_t a_splitCount) const;
    void PushShadowTextureArray(uint32_t a_slot, uint32_t a_dirLightAddr) const;
    void BindRenderTexture(uint32_t a_addr, e_RenderTextureBindMode a_bindMode) const;
    void BlitRTRT(uint32_t a_srcAddr, uint32_t a_dstAddr) const;
    void DrawMaterial();
    void DrawModel(const glm::mat4& a_transform, uint32_t a_addr);

    void SetLightSplits(const LightShadowSplit* a_splits, uint32_t a_splitCount) const;
};

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