#pragma once

#ifdef CUBE_LANGUAGE_CSHARP

#define IOP_STRING string
#define IOP_VEC2 Vector2
#define IOP_VEC3 Vector3
#define IOP_VEC4 Vector4
#define IOP_UINT16 ushort
#define IOP_UINT32 uint
#define IOP_ARRAY Array

#define IOP_CSPUBLIC public

#define IOP_ENUM_NAME(name) name
#define IOP_ENUM_VALUE(enum, value) value
#define IOP_STRUCTURE_END

#else
#include <cstdint>
#include <mono/jit/jit.h>

#define IOP_STRING MonoString*
#define IOP_VEC2 glm::vec2
#define IOP_VEC3 glm::vec3
#define IOP_VEC4 glm::vec4
#define IOP_UINT16 uint16_t
#define IOP_UINT32 uint32_t
#define IOP_ARRAY MonoArray*

#define IOP_CSPUBLIC 

#define IOP_ENUM_NAME(name) e_##name
#define IOP_ENUM_VALUE(enum, value) enum##_##value
#define IOP_STRUCTURE_END ;

#endif