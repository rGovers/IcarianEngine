// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "Core/DataTypes/Allocators/BlockAllocator.h"
#include "Core/DataTypes/Allocators/TrackerAllocator.h"
#include "Core/DataTypes/Array.h"
#include "Core/DataTypes/COWString.h"
#include "Core/DataTypes/Dictionary.h"

class Config;
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
    static constexpr uint32_t SmallAllocatorSize = 32 << 10;
    static constexpr uint32_t LargeAllocatorSize = 32 << 20;
    static constexpr bool EnableLeakTracking = false;

    IcarianCore::BlockAllocator*                                                 m_smallAllocator;
    IcarianCore::BlockAllocator*                                                 m_largeAllocator;

    IcarianCore::TrackerAllocator*                                               m_trackerAllocator;

    IcarianCore::Array<IcarianCore::Allocator*>*                                 m_allocatorChain;

    MonoAllocatorVTable                                                          m_allocatorTable;

    IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String>* m_dllLookup;

    MonoDomain*                                                                  m_domain;
    MonoAssembly*                                                                m_assembly;

    MonoImage*                                                                   m_image;

    MonoClass*                                                                   m_programClass;

    MonoMethod*                                                                  m_initMethod;
    MonoMethod*                                                                  m_updateMethod;
    MonoMethod*                                                                  m_lateUpdateMethod;
    MonoMethod*                                                                  m_shutdownMethod;

protected:

public:
    RuntimeManager(Config* a_config);
    ~RuntimeManager();

    static void Init(Config* a_config);
    static void Destroy();

    static void BindFunction(const char* a_location, void* a_function);
    static void BindFunction(const IcarianCore::COWU8String& a_location, void* a_function);

    static void Exec(int32_t a_argc, char* a_argv[]);
    static void Update(double a_delta, double a_time);
    static void LateUpdate();

    static void AttachThread();

    static void PushDLLPath(const char* a_path);
    static void PushDLLPath(const IcarianCore::COWU8String& a_path);
    static IcarianCore::COWU8String GetDLLPath(const char* a_path);
    static IcarianCore::COWU8String GetDLLPath(const IcarianCore::COWU8String& a_name);

    static MonoDomain* GetDomain();

    static MonoClass* GetClass(const char* a_namespace, const char* a_name);
    static MonoClass* GetClass(const IcarianCore::COWU8String& a_namespace, const IcarianCore::COWU8String& a_name);

    static RuntimeFunction* GetFunction(const char* a_namespace, const char* a_class, const char* a_method);
    static RuntimeFunction* GetFunction
    (
        const IcarianCore::COWU8String& a_namespace,
        const IcarianCore::COWU8String& a_class,
        const IcarianCore::COWU8String& a_method
    );
};

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
