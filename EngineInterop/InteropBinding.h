#pragma once

#ifdef CUBE_LANGUAGE_CSHARP
#define IOP_BIND_FUNCTION(ret, nspace, klass, name, code, ...) namespace nspace { internal partial class klass { [MethodImpl(MethodImplOptions.InternalCall)]internal extern static ret name(__VA_ARGS__); } }
#endif