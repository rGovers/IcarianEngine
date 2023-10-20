#pragma once

#ifdef CUBE_LANGUAGE_CSHARP

#define IOP_STRING string
#define IOP_VEC2 Vector2
#define IOP_VEC3 Vector3
#define IOP_VEC4 Vector4
#define IOP_UINT8 byte
#define IOP_UINT16 ushort
#define IOP_UINT32 uint
#define IOP_ARRAY Array

#define IOP_PACKED [StructLayout(LayoutKind.Sequential, Pack = 0)]

#define IOP_UINT8_MAX byte.MaxValue
#define IOP_UINT16_MAX ushort.MaxValue
#define IOP_UINT32_MAX uint.MaxValue

#define IOP_POINTER(type) IntPtr

#define IOP_CSPUBLIC public
#define IOP_CSINTERNAL internal

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
#define IOP_UINT8 uint8_t
#define IOP_UINT16 uint16_t
#define IOP_UINT32 uint32_t
#define IOP_ARRAY MonoArray*

#define IOP_PACKED

#define IOP_UINT8_MAX UINT8_MAX
#define IOP_UINT16_MAX UINT16_MAX
#define IOP_UINT32_MAX UINT32_MAX

#define IOP_POINTER(type) type

#define IOP_CSPUBLIC 
#define IOP_CSINTERNAL

#define IOP_ENUM_NAME(name) e_##name
#define IOP_ENUM_VALUE(enum, value) enum##_##value
#define IOP_STRUCTURE_END ;

#endif