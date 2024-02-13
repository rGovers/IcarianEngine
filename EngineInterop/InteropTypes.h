#pragma once

// DO NOT PUT ## in the preprocessor has a fit as it sees it as a macro otherwise
#define IOP_MACRO_ESCAPE(hash, macro) hash macro
// Cannot put # in a macro need to be pulled from a define and expanded to work otherwise preprocessor has a fit
#define IOP_EXPAND_MACRO(str) str
#define IOP_MACRO_HASH #

#ifdef CUBE_LANGUAGE_CSHARP

// So you can use the preprocess to export macros however needs to be done in a very specific way in order for the preprocessor to not have a fit about the bullshit that is occuring
// The preprocessor seems to be very careful about creating macros in macros so have to bury it under several layers of abstraction to smuggle it throught the preprocessor
// But running the preprocessor in a loop should allow mutable state and use as a full programming language theoretically need to investigate for a fun project
#define IOP_CSMACRO(macro) IOP_MACRO_ESCAPE(IOP_EXPAND_MACRO(IOP_MACRO_HASH), macro)

#define IOP_STRING string
#define IOP_VEC2 Vector2
#define IOP_VEC3 Vector3
#define IOP_VEC4 Vector4
#define IOP_QUAT Quaternion
#define IOP_IVEC2 IVector2
#define IOP_IVEC3 IVector3
#define IOP_IVEC4 IVector4
#define IOP_UINT8 byte
#define IOP_UINT16 ushort
#define IOP_UINT32 uint
#define IOP_ARRAY(type) type

#define IOP_PACKED [StructLayout(LayoutKind.Sequential, Pack = 0)]

#define IOP_UINT8_MAX byte.MaxValue
#define IOP_UINT16_MAX ushort.MaxValue
#define IOP_UINT32_MAX uint.MaxValue

#define IOP_POINTER(type) IntPtr

#define IOP_CSPUBLIC public
#define IOP_CSINTERNAL internal

#define IOP_CONSTEXPR readonly

#define IOP_ENUM_NAME(name) name
#define IOP_ENUM_VALUE(enum, value) value
#define IOP_STRUCTURE_END

#else
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <cstdint>

// C++ so ignore C# macros
#define IOP_CSMACRO(macro) 

#define IOP_STRING MonoString*
#define IOP_VEC2 glm::vec2
#define IOP_VEC3 glm::vec3
#define IOP_VEC4 glm::vec4
#define IOP_QUAT glm::quat
#define IOP_IVEC2 glm::ivec2
#define IOP_IVEC3 glm::ivec3
#define IOP_IVEC4 glm::ivec4
#define IOP_UINT8 uint8_t
#define IOP_UINT16 uint16_t
#define IOP_UINT32 uint32_t
#define IOP_ARRAY(type) MonoArray*

#define IOP_PACKED

#define IOP_UINT8_MAX UINT8_MAX
#define IOP_UINT16_MAX UINT16_MAX
#define IOP_UINT32_MAX UINT32_MAX

#define IOP_POINTER(type) type

#define IOP_CSPUBLIC 
#define IOP_CSINTERNAL

#define IOP_CONSTEXPR constexpr

#define IOP_ENUM_NAME(name) e_##name
#define IOP_ENUM_VALUE(enum, value) enum##_##value
#define IOP_STRUCTURE_END ;

#endif