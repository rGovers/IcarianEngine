// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Runtime/RuntimeManager.h"

#include <cstring>
#include <filesystem>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <mono/utils/mono-dl-fallback.h>

#include "Core/IcarianDefer.h"
#include "IcarianError.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Runtime/RuntimeFunction.h"

static RuntimeManager* Instance = nullptr;

#include "EngineIcarianAssemblyInterop.h"

ENGINE_ICARIANASSEMBLY_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

#ifndef WIN32
#include <dlfcn.h>

#include "Core/MonoNativeImpl.h"

static constexpr char MonoNativeLibName[] = "libmono-native.so";
static constexpr uint32_t MonoNativeLibNameLength = sizeof(MonoNativeLibName) - 1;
// Ludum Dare 54 hack 
static constexpr char MonoNativeBaseName[] = "System.Native";
static constexpr uint32_t MonoNativeBaseNameLength = sizeof(MonoNativeBaseName) - 1;

#define MonoThisLibHandle ((void*)-1)
#else
#include "Core/WindowsHeaders.h"
#endif

static void* RuntimeDLOpen(const char* a_name, int a_flags, char** a_error, void* a_userData)
{
    const std::filesystem::path p = RuntimeManager::GetDLLPath(a_name);

#ifdef WIN32
    if (!p.empty())
    {
        const std::filesystem::path ext = p.extension();
        
        if (ext == ".dll")
        {
            const std::string str = p.string();

            return LoadLibraryA(str.c_str());
        }   
    }
#else
    if (!p.empty())
    {
        const std::filesystem::path ext = p.extension();

        if (ext == ".so")
        {
            const std::string str = p.string();

            void* handle = dlopen(str.c_str(), a_flags);
            if (handle == NULL)
            {
                IERROR(std::string("Failed to open DLL: ") + dlerror());
            }

            return handle; 
        }
    }

    const uint32_t len = (uint32_t)strlen(a_name);
    const char* ptrLib = a_name + len - MonoNativeLibNameLength;
    const char* ptrBase = a_name + len - MonoNativeBaseNameLength;

    if ((len > MonoNativeLibNameLength && strcmp(ptrLib, MonoNativeLibName) == 0) || (len > MonoNativeBaseNameLength && strcmp(ptrBase, MonoNativeBaseName) == 0))
    {
        return MonoThisLibHandle;
    }
#endif

    return NULL;
}

static void* RuntimeDLSymbol(void* a_handle, const char* a_name, char** a_error, void* a_userData)
{
#ifdef WIN32
    if (a_handle != NULL)
    {
        return (void*)GetProcAddress((HMODULE)a_handle, a_name);
    }
#else
    if (a_handle == MonoThisLibHandle)
    {
        return IcarianCore::MonoNativeImpl::GetFunction(a_name);
    }

    if (a_handle != NULL)
    {
        return dlsym(a_handle, a_name);
    }
#endif

    return NULL;
}

RuntimeManager::RuntimeManager()
{
    mono_config_parse(NULL);

    const std::filesystem::path currentDir = std::filesystem::current_path();

    const std::filesystem::path libDir = currentDir / "lib";
    const std::filesystem::path etcDir = currentDir / "etc";

    mono_set_dirs(libDir.string().c_str(), etcDir.string().c_str());
    
#ifndef WIN32
    IcarianCore::MonoNativeImpl::Init();
#endif

    mono_dl_fallback_register(RuntimeDLOpen, RuntimeDLSymbol, NULL, NULL);

    m_domain = mono_jit_init_version("Core", "v4.0");
    m_assembly = mono_domain_assembly_open(m_domain, "IcarianCS.dll");
    IVERIFY(m_assembly != NULL);

    m_image = mono_assembly_get_image(m_assembly);
    IVERIFY(m_image != NULL);
    m_programClass = mono_class_from_name(m_image, "IcarianEngine", "Program");
    IVERIFY(m_programClass != NULL);

    MonoMethodDesc* initDesc = mono_method_desc_new(":Init(string[])", 0);
    IVERIFY(initDesc != NULL);
    IDEFER(mono_method_desc_free(initDesc));
    m_initMethod = mono_method_desc_search_in_class(initDesc, m_programClass);
    IVERIFY(m_initMethod != NULL);

    MonoMethodDesc* updateDesc = mono_method_desc_new(":Update(double,double)", 0);
    IVERIFY(updateDesc != NULL);
    IDEFER(mono_method_desc_free(updateDesc));
    m_updateMethod = mono_method_desc_search_in_class(updateDesc, m_programClass);
    IVERIFY(m_updateMethod != NULL);

    MonoMethodDesc* lateUpdateDesc = mono_method_desc_new(":LateUpdate()", 0);
    IVERIFY(lateUpdateDesc != NULL);
    IDEFER(mono_method_desc_free(lateUpdateDesc));
    m_lateUpdateMethod = mono_method_desc_search_in_class(lateUpdateDesc, m_programClass);
    IVERIFY(m_lateUpdateMethod != NULL);

    MonoMethodDesc* shutdownDesc = mono_method_desc_new(":Shutdown()", 0);
    IVERIFY(shutdownDesc != NULL);
    IDEFER(mono_method_desc_free(shutdownDesc));
    m_shutdownMethod = mono_method_desc_search_in_class(shutdownDesc, m_programClass);
    IVERIFY(m_shutdownMethod != NULL);

    ENGINE_ICARIANASSEMBLY_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
RuntimeManager::~RuntimeManager()
{
    mono_runtime_invoke(m_shutdownMethod, NULL, NULL, NULL);

    mono_free_method(m_initMethod);
    mono_free_method(m_updateMethod);
    mono_free_method(m_lateUpdateMethod);
    mono_free_method(m_shutdownMethod);

    mono_jit_cleanup(m_domain);

    mono_dl_fallback_unregister(NULL);
    
#ifndef WIN32
    IcarianCore::MonoNativeImpl::Destroy();
#endif
}

void RuntimeManager::Init()
{
    if (Instance == nullptr)
    {
        Instance = new RuntimeManager();
    }
}
void RuntimeManager::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void RuntimeManager::Exec(int a_argc, char* a_argv[])
{
    MonoClass* stringClass = mono_get_string_class();

    MonoArray* argsArr = mono_array_new(Instance->m_domain, stringClass, (uintptr_t)a_argc);
    for (int i = 0; i < a_argc; ++i)
    {
        mono_array_set(argsArr, MonoString*, i, mono_string_new(Instance->m_domain, a_argv[i]));
    }

    void* args[] =
    {
        argsArr
    };

    mono_runtime_invoke(Instance->m_initMethod, NULL, args, NULL);
}
void RuntimeManager::Update(double a_delta, double a_time)
{
    PROFILESTACK("Runtime Update");
    
    void* args[] =
    {
        &a_delta,
        &a_time
    };

    mono_runtime_invoke(Instance->m_updateMethod, NULL, args, NULL);
}
void RuntimeManager::LateUpdate()
{
    PROFILESTACK("Runtime Late Update");

    mono_runtime_invoke(Instance->m_lateUpdateMethod, NULL, NULL, NULL);
}

void RuntimeManager::BindFunction(const std::string_view& a_location, void* a_function)
{
    mono_add_internal_call(a_location.data(), a_function);
}

void RuntimeManager::AttachThread()
{
    mono_jit_thread_attach(Instance->m_domain);
}

void RuntimeManager::PushDLLPath(const std::filesystem::path& a_path)
{
    const std::filesystem::path filename = a_path.filename();

    Instance->m_dllLookup.emplace(filename.string(), a_path);
}
std::filesystem::path RuntimeManager::GetDLLPath(const std::string_view& a_path)
{
    if (Instance == nullptr)
    {
        return std::filesystem::path();
    }

    const std::filesystem::path assemblyPath = std::filesystem::path(a_path);
    const std::filesystem::path filename = assemblyPath.filename();
    const std::string s = filename.string();

    auto iter = Instance->m_dllLookup.find(s);
    if (iter != Instance->m_dllLookup.end())
    {
        return iter->second;
    }

#ifdef WIN32
    iter = Instance->m_dllLookup.find(s + ".dll");
    if (iter != Instance->m_dllLookup.end())
    {
        return iter->second;
    }
#else
    iter = Instance->m_dllLookup.find(s + ".so");
    if (iter != Instance->m_dllLookup.end())
    {
        return iter->second;
    }

    iter = Instance->m_dllLookup.find("lib" + s);
    if (iter != Instance->m_dllLookup.end())
    {
        return iter->second;
    }

    iter = Instance->m_dllLookup.find("lib" + s + ".so");
    if (iter != Instance->m_dllLookup.end())
    {
        return iter->second;
    }
#endif

    return std::filesystem::path();
}

MonoDomain* RuntimeManager::GetDomain()
{
    return Instance->m_domain;
}

MonoClass* RuntimeManager::GetClass(const std::string_view& a_namespace, const std::string_view& a_name)
{
    return mono_class_from_name(Instance->m_image, a_namespace.data(), a_name.data());
}

RuntimeFunction* RuntimeManager::GetFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method)
{
    MonoClass* cls = mono_class_from_name(Instance->m_image, a_namespace.data(), a_class.data());
    IVERIFY(cls != NULL);

    MonoMethodDesc* desc = mono_method_desc_new(a_method.data(), 0);
    IVERIFY(desc != NULL);
    IDEFER(mono_method_desc_free(desc));
    MonoMethod* method = mono_method_desc_search_in_class(desc, cls);
    IVERIFY(method != NULL);

    return new RuntimeFunction(method);
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