#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#define GLSL_DEFINITION(name) uniform name
#define F_DEFINITION(name) struct name

#define GLSL_MAT4(name) mat4 name;
#define F_MAT4(name) alignas(16) glm::mat4 name;

#define GLSL_FLOAT(name)  float name;
#define GLSL_VEC2(name) vec2 name;
#define GLSL_VEC3(name) vec3 name;
#define GLSL_VEC4(name) vec4 name;
#define F_FLOAT(name) alignas(16) float name;
#define F_VEC2(name) alignas(16) glm::vec2 name;
#define F_VEC3(name) alignas(16) glm::vec3 name;
#define F_VEC4(name) alignas(16) glm::vec4 name;

#define SHADER_UNIFORM_STR(S) #S
#define GLSL_UNIFORM_STRING(set, location, name, structure) std::string("layout(binding=") + (set) + ",set=" + (location) + ") " SHADER_UNIFORM_STR(structure) " " + (name) + ";" 
#define GLSL_PUSHBUFFER_STRING(name, structure) std::string("layout(push_constant) " SHADER_UNIFORM_STR(structure) " ") + (name) + ";"

#define CAMERA_SHADER_STRUCTURE(D, M4) \
D(CameraShaderBuffer) \
{ \
M4(View) \
M4(Proj) \
M4(InvView) \
M4(InvProj) \
M4(ViewProj) \
}
#define GLSL_CAMERA_SHADER_STRUCTURE CAMERA_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)

#define DIRECTIONAL_LIGHT_SHADER_STRUCTURE(D, V4) \
D(DirectionalLightShaderBuffer) \
{ \
V4(LightDir) \
V4(LightColor) \
}
#define GLSL_DIRECTIONAL_LIGHT_SHADER_STRUCTURE DIRECTIONAL_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_VEC4)

#define POINT_LIGHT_SHADER_STRUCTURE(D, FL, V4) \
D(PointLightShaderBuffer) \
{ \
V4(LightPos) \
V4(LightColor) \
FL(Radius) \
}
#define GLSL_POINT_LIGHT_SHADER_STRUCTURE POINT_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_FLOAT, GLSL_VEC4)

#define SPOT_LIGHT_SHADER_STRUCTURE(D, V3, V4) \
D(SpotLightShaderBuffer) \
{ \
V3(LightPos) \
V4(LightDir) \
V4(LightColor) \
V3(CutoffAngle) \
}
#define GLSL_SPOT_LIGHT_SHADER_STRUCTURE SPOT_LIGHT_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_VEC3, GLSL_VEC4)

#define MODEL_SHADER_STRUCTURE(D, M4) \
D(ModelShaderBuffer) \
{ \
M4(Model) \
M4(InvModel) \
}
#define GLSL_MODEL_SHADER_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)

#define TIME_SHADER_BUFFER(D, V2) \
D(TimeShaderBuffer) \
{ \
V2(Time) \
}
#define GLSL_TIME_SHADER_STRUCTURE TIME_SHADER_BUFFER(GLSL_DEFINITION, GLSL_VEC2)

CAMERA_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
DIRECTIONAL_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_VEC4);
POINT_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_FLOAT, F_VEC4);
SPOT_LIGHT_SHADER_STRUCTURE(F_DEFINITION, F_VEC3, F_VEC4);
MODEL_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
TIME_SHADER_BUFFER(F_DEFINITION, F_VEC2);
