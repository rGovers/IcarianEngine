// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <filesystem>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string_view>
#include <unordered_map>

class RenderEngine;
class RuntimeFunction;

#define FLARE_MONO_EXPORT(ret, func, ...) static ret func(__VA_ARGS__)

#define RUNTIME_FUNCTION_NAME(klass, name) MRF_##klass##_##name
#define RUNTIME_FUNCTION_STRING(namespace, klass, name) #namespace "." #klass "::" #name

#define RUNTIME_FUNCTION(ret, klass, name, code, ...) FLARE_MONO_EXPORT(ret, RUNTIME_FUNCTION_NAME(klass, name), __VA_ARGS__) code

#define RUNTIME_FUNCTION_DEFINITION(ret, namespace, klass, name, code, ...) RUNTIME_FUNCTION(ret, klass, name, code, __VA_ARGS__)
#define RUNTIME_FUNCTION_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(namespace, klass, name);

#define BIND_FUNCTION(namespace, klass, name) RuntimeManager::BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name))

class RuntimeManager
{
private:
    std::unordered_map<std::string, std::filesystem::path> m_dllLookup;

    MonoDomain*                                            m_domain;
    MonoAssembly*                                          m_assembly;
                                        
    MonoImage*                                             m_image;
                                        
    MonoClass*                                             m_programClass;
                                        
    MonoMethod*                                            m_initMethod;
    MonoMethod*                                            m_updateMethod;
    MonoMethod*                                            m_lateUpdateMethod;
    MonoMethod*                                            m_shutdownMethod;

    RuntimeManager();
protected:

public:
    ~RuntimeManager();

    static void Init();
    static void Destroy();

    static void BindFunction(const std::string_view& a_location, void* a_function);

    static void Exec(int32_t a_argc, char* a_argv[]);
    static void Update(double a_delta, double a_time);
    static void LateUpdate();

    static void AttachThread();

    static void PushDLLPath(const std::filesystem::path& a_path);
    static std::filesystem::path GetDLLPath(const std::string_view& a_name);

    static MonoDomain* GetDomain();

    static MonoClass* GetClass(const std::string_view& a_namespace, const std::string_view& a_name);

    static RuntimeFunction* GetFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method);
};

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