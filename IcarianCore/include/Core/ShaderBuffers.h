// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#define SHADER_UNIFORM_STR(S) #S

#define GLSL_DEFINITION(name) struct name##Data
#define GLSL_SSBO_DEFINITION(name) struct name##Data
#define GLSL_PUSH_DEFINITION(name) uniform name
#define F_DEFINITION(name) struct Shader##name

#define GLSL_MAT4(name) mat4 name;
#define F_MAT4(name) alignas(16) glm::mat4 name;

#define GLSL_FLOAT(name) float name;
#define GLSL_VEC2(name) vec2 name;
#define GLSL_VEC3(name) vec3 name;
#define GLSL_VEC4(name) vec4 name;
#define F_FLOAT(name) alignas(16) float name;
#define F_VEC2(name) alignas(16) glm::vec2 name;
#define F_VEC3(name) alignas(16) glm::vec3 name;
#define F_VEC4(name) alignas(16) glm::vec4 name;

#define GLSL_VULKAN_UNIFORM_STRING(slot, name, structure, structureName) std::string(SHADER_UNIFORM_STR(structure)) + "; layout(std140,binding=" + (slot) + ",set=" + (slot) + ") uniform " + (structureName) + "{ " + (structureName) + "Data " + (name) + "; };" 
#define GLSL_VULKAN_SSBO_STRING(slot, name, structure, structureName) std::string(SHADER_UNIFORM_STR(structure)) + "; layout(std140,binding=" + (slot) + ",set=" + (slot) + ") readonly buffer " + (structureName) + " { int Count; " + (structureName) + "Data objects[]; } " + (name) + ";" 
#define GLSL_VULKAN_PUSHBUFFER_STRING(name, structure) std::string("layout(push_constant) " SHADER_UNIFORM_STR(structure) " ") + (name) + ";"

#define GLSL_OPENGL_UNIFORM_STRING(slot, name, structure, structureName) std::string(SHADER_UNIFORM_STR(structure)) + "; layout(std140,binding=" + (slot) + ") uniform " + (structureName) + " { " + (structureName) + "Data " + (name) + "; };"
#define GLSL_OPENGL_SSBO_STRING(slot, name, structure, structureName) std::string(SHADER_UNIFORM_STR(structure)) + "; layout(std140,binding=" + (slot) + ") readonly buffer " + (structureName) + " { int Count; " + (structureName) + "Data objects[]; } " + (name) + ";" 
#define GLSL_OPENGL_PUSHBUFFER_STRING(name, structure) std::string("layout(binding=64,std140) " SHADER_UNIFORM_STR(structure) " ") + (name) + ";"

#define CAMERA_SHADER_STRUCTURE(D, M4) \
D(CameraBuffer) \
{ \
M4(View) \
M4(Proj) \
M4(InvView) \
M4(InvProj) \
M4(ViewProj) \
}
#define GLSL_CAMERA_SHADER_STRUCTURE CAMERA_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)

#define PARTICLE_SHADER_STRUCTURE(D, V3, V4) \
D(ParticleBuffer) \
{ \
V4(Position) \
V3(Velocity) \
V4(Color) \
}
#define GLSL_PARTICLE_SSBO_STRUCTURE PARTICLE_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_VEC3, GLSL_VEC4)

#define SHADOW_LIGHT_SHADER_STRUCTURE(D, M4, F) \
D(ShadowLightBuffer) \
{ \
M4(LVP) \
F(Split) \
}
#define GLSL_SHADOW_LIGHT_SHADER_STRUCTURE SHADOW_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4, GLSL_FLOAT)
#define GLSL_SHADOW_LIGHT_PUSH_STRUCTURE SHADOW_LIGHT_SHADER_STRUCTURE(GLSL_PUSH_DEFINITION, GLSL_MAT4, GLSL_FLOAT)
#define GLSL_SHADOW_LIGHT_SSBO_STRUCTURE SHADOW_LIGHT_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_MAT4, GLSL_FLOAT)

#define AMBIENT_LIGHT_SHADER_STRUCTURE(D, V4) \
D(AmbientLightBuffer) \
{ \
V4(LightColor) \
}
#define GLSL_AMBIENT_LIGHT_SHADER_STRUCTURE AMBIENT_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_VEC4)
#define GLSL_AMBIENT_LIGHT_SSBO_STRUCTURE AMBIENT_LIGHT_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_VEC4)

#define DIRECTIONAL_LIGHT_SHADER_STRUCTURE(D, V4) \
D(DirectionalLightBuffer) \
{ \
V4(LightDir) \
V4(LightColor) \
}
#define GLSL_DIRECTIONAL_LIGHT_SHADER_STRUCTURE DIRECTIONAL_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_VEC4)
#define GLSL_DIRECTIONAL_LIGHT_SSBO_STRUCTURE DIRECTIONAL_LIGHT_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_VEC4)

#define POINT_LIGHT_SHADER_STRUCTURE(D, FL, V4) \
D(PointLightBuffer) \
{ \
V4(LightPos) \
V4(LightColor) \
FL(Radius) \
}
#define GLSL_POINT_LIGHT_SHADER_STRUCTURE POINT_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_FLOAT, GLSL_VEC4)
#define GLSL_POINT_LIGHT_SSBO_STRUCTURE POINT_LIGHT_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_FLOAT, GLSL_VEC4)

#define SPOT_LIGHT_SHADER_STRUCTURE(D, V3, V4) \
D(SpotLightBuffer) \
{ \
V3(LightPos) \
V4(LightDir) \
V4(LightColor) \
V3(CutoffAngle) \
}
#define GLSL_SPOT_LIGHT_SHADER_STRUCTURE SPOT_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_VEC3, GLSL_VEC4)
#define GLSL_SPOT_LIGHT_SSBO_STRUCTURE SPOT_LIGHT_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_VEC3, GLSL_VEC4)

#define MODEL_SHADER_STRUCTURE(D, M4) \
D(ModelBuffer) \
{ \
M4(Model) \
M4(InvModel) \
}
#define GLSL_MODEL_SHADER_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)
#define GLSL_MODEL_PUSH_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_PUSH_DEFINITION, GLSL_MAT4)
#define GLSL_MODEL_SSBO_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_MAT4)

#define BONE_SHADER_STRUCTURE(D, M4) \
D(BoneBuffer) \
{ \
M4(BoneMatrix) \
}
#define GLSL_BONE_SHADER_STRUCTURE BONE_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)
#define GLSL_BONE_SSBO_STRUCTURE BONE_SHADER_STRUCTURE(GLSL_SSBO_DEFINITION, GLSL_MAT4)

#define UI_SHADER_STRUCTURE(D, V4) \
D(UIBuffer) \
{ \
V4(Color) \
}
#define GLSL_UI_SHADER_STRUCTURE UI_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_VEC4)
#define GLSL_UI_PUSH_STRUCTURE UI_SHADER_STRUCTURE(GLSL_PUSH_DEFINITION, GLSL_VEC4)

#define TIME_SHADER_BUFFER(D, V2) \
D(TimeBuffer) \
{ \
V2(Time) \
}
#define GLSL_TIME_SHADER_STRUCTURE TIME_SHADER_BUFFER(GLSL_DEFINITION, GLSL_VEC2)

namespace IcarianCore 
{
    CAMERA_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
    PARTICLE_SHADER_STRUCTURE(F_DEFINITION, F_VEC3, F_VEC4);
    SHADOW_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_MAT4, F_FLOAT);
    AMBIENT_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_VEC4);
    DIRECTIONAL_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_VEC4);
    POINT_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_FLOAT, F_VEC4);
    SPOT_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_VEC3, F_VEC4);
    MODEL_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
    BONE_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
    UI_SHADER_STRUCTURE(F_DEFINITION, F_VEC4);
    TIME_SHADER_BUFFER(F_DEFINITION, F_VEC2);
}

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