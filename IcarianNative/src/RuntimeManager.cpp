// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Runtime/RuntimeManager.h"

#include <cstring>
#include <filesystem>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <mono/utils/mono-dl-fallback.h>

#include "Config.h"
#include "Core/DataTypes/Allocators/BlockAllocator.h"
#include "Core/DataTypes/Allocators/LeakAllocator.h"
#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/Allocators/MultiSourceAllocator.h"
#include "Core/DataTypes/Allocators/OSAllocator.h"
#include "Core/DataTypes/Allocators/TrackerAllocator.h"
#include "Core/DataTypes/Allocators/UberAllocator.h"
#include "Core/IcarianDefer.h"
#include "Core/StringUtils.h"
#include "IcarianError.h"
#include "IO.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Runtime/RuntimeFunction.h"

static IcarianCore::ComplexAllocator* Alloc = nullptr;
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
    const IcarianCore::COWU8String path = RuntimeManager::GetDLLPath(a_name);

#ifdef WIN32
    if (!path.Empty())
    {
        const IcarianCore:: ext = IO::GetExtension(path, Alloc);

        if (ext == ".dll")
        {
            return LoadLibraryA(path.CStr());
        }
    }
#else
    if (!path.Empty())
    {
        const IcarianCore::COWU8String ext = IO::GetExtension(path, Alloc);

        if (ext == ".so")
        {
            void* handle = dlopen(path.CStr(), a_flags);
            if (handle == NULL)
            {
                IERROR(IcarianCore::COWU8String("Failed to open DLL: ", IcarianCore::MallocAllocator::Instance) + dlerror());
            }

            return handle;
        }
    }

    const uint32_t len = ILAMBDA(
    {
        const char* slider = a_name;
        while (*slider != 0)
        {
            ++slider;
        }

        ILRETURN (uint32_t)(slider - a_name);
    });
    const char* ptrLib = a_name + len - MonoNativeLibNameLength;
    const char* ptrBase = a_name + len - MonoNativeBaseNameLength;

    const bool isNative = len > MonoNativeLibNameLength && strcmp(ptrLib, MonoNativeLibName) == 0;
    const bool isBaseNative = len > MonoNativeBaseNameLength && strcmp(ptrBase, MonoNativeBaseName) == 0;
    if (isNative || isBaseNative)
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

constexpr static uint32_t RuntimeDefaultAllocationAlignment = 16;

static void* Mono_Malloc(size_t a_size)
{
    return Alloc->Allocate((uint64_t)a_size, RuntimeDefaultAllocationAlignment);
}
static void Mono_Free(void* a_ptr)
{
    Alloc->Free(a_ptr);
}
static void* Mono_Realloc(void* a_ptr, size_t a_count)
{
    return Alloc->Realloc(a_ptr, a_count, RuntimeDefaultAllocationAlignment);
}
static void* Mono_Calloc(size_t a_count, size_t a_size)
{
    // C Spec is weird about 0 sized allocations and seems to be implementation dependent in the real world so *shrugs*
    if (a_count == 0)
    {
        return nullptr;
    }

    if (a_size == 0)
    {
        return nullptr;
    }

    // Urgh just looked at the spec and alignment is a requirement and Mono wants us to be a malloc allocator
    // This is dumb as we are returning memory larger then was requested worst case N * (RuntimeDefaultAllocationAlignment - 1) bytes extra
    // Normally would look at the library to check if the elements need to be aligned but this is for a C# runtime so can make no assurances so just follow the spec for safety
    const uint64_t alignedSize = IcarianCore::AlignTo((uint64_t)a_size, RuntimeDefaultAllocationAlignment);
    const uint64_t finalSize = (uint64_t)a_count * alignedSize;
    return Alloc->ZAllocate(finalSize, RuntimeDefaultAllocationAlignment);
}

RuntimeManager::RuntimeManager(Config* a_config)
{
    IVERIFY(Alloc == nullptr);
    m_smallAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::BlockAllocator>(SmallAllocatorSize, IcarianCore::UberAllocator::Instance);
    m_largeAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::BlockAllocator>(LargeAllocatorSize, IcarianCore::UberAllocator::Instance);

    m_allocatorChain = m_smallAllocator->Create<IcarianCore::Array<IcarianCore::Allocator*>>(m_smallAllocator);

    const IcarianCore::AllocationSource allocatorSources[] =
    {
        {
            .Alloc = m_smallAllocator,
            .MaxSize = SmallAllocatorSize >> 1,
        },
        {
            .Alloc = m_largeAllocator,
            .MaxSize = LargeAllocatorSize >> 1,
        },
        {
            .Alloc = IcarianCore::OSAllocator::Instance,
            .MaxSize = uint64_t(-1),
        },
    };

    constexpr uint32_t AllocatorCount = sizeof(allocatorSources) / sizeof(*allocatorSources);

    Alloc = m_smallAllocator->Create<IcarianCore::MultiSourceAllocator>(m_smallAllocator, allocatorSources, AllocatorCount);
    m_allocatorChain->Push(Alloc);

    if (a_config->IsHeadless())
    {
        m_trackerAllocator = m_smallAllocator->Create<IcarianCore::TrackerAllocator>(Alloc);
        Alloc = m_trackerAllocator;
        m_allocatorChain->Push(Alloc);
    }

#ifdef DEBUG
    if constexpr (EnableLeakTracking)
    {
        Alloc = m_smallAllocator->Create<IcarianCore::LeakAllocator>(Alloc);

        m_allocatorChain->Push(Alloc);
    }
#endif

    m_dllLookup = Alloc->Create<IcarianCore::Dictionary<IcarianCore::COWU8String, IcarianCore::COWU8String>>(Alloc);

    m_allocatorTable =
    {
        .version = MONO_ALLOCATOR_VTABLE_VERSION,
        .malloc = Mono_Malloc,
        .realloc = Mono_Realloc,
        .free = Mono_Free,
        .calloc = Mono_Calloc,
    };

    mono_set_allocator_vtable(&m_allocatorTable);

    mono_config_parse(NULL);

    const std::filesystem::path currentDir = std::filesystem::current_path();

    const std::filesystem::path libDir = currentDir / "lib";
    const std::filesystem::path etcDir = currentDir / "etc";

    const std::string libStr = libDir.generic_string();
    const std::string etcStr = etcDir.generic_string();

    mono_set_dirs(libStr.c_str(), etcStr.c_str());

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

    Alloc->Destroy(m_dllLookup);

    // Well gave Mono a custom allocator and got good news, bad news and good news
    // Good News. ASan is no longer complaining about a memory leak
    // Bad News. Our allocator is now complaining about a memory leak and ASan can no longer see it as we are handling the memory
    // Good News. I now know which system has the leak
    // That is our allocator and ASan against Mono and the reports match so I think Mono has a memory leak
    // UPDATE: Welp after sifting through the LeakAllocator Mono is a leaky sieve there is no single leak
    // It is gonna be a project and a half to fix them all therefore *plugs ears* LALALALALALA CANNOT HEAR YOU
    // In all seriousness this may be a ignore the issue until I find a couple months to spare to write a C# runtime
    // I am not gonna put effort into a deprecated runtime
    IVERIFY(Alloc != nullptr);

    const uint32_t allocatorChainSize = m_allocatorChain->Size();
    for (uint32_t i = 0; i < allocatorChainSize; ++i)
    {
        IcarianCore::Allocator* alloc = (*m_allocatorChain)[allocatorChainSize - i - 1];
        m_smallAllocator->Destroy(alloc);
    }

    m_smallAllocator->Destroy(m_allocatorChain);

    IcarianCore::MallocAllocator::Instance->Destroy(m_largeAllocator);
    IcarianCore::MallocAllocator::Instance->Destroy(m_smallAllocator);
}

void RuntimeManager::Init(Config* a_config)
{
    if (Instance == nullptr)
    {
        Instance = IcarianCore::MallocAllocator::Instance->Create<RuntimeManager>(a_config);
    }
}
void RuntimeManager::Destroy()
{
    if (Instance != nullptr)
    {
        IcarianCore::MallocAllocator::Instance->Destroy(Instance);
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

    if (Instance->m_trackerAllocator != nullptr)
    {
        const uint64_t size = Instance->m_trackerAllocator->GetMemoryUsage();

        Profiler::PushMemoryFrame(ProfilerMemoryFrame_CSharp, size);
    }

    void* args[] =
    {
        &a_delta,
        &a_time
    };

    mono_runtime_invoke(Instance->m_updateMethod, NULL, args, NULL);

    {
        PROFILESTACK("Runtime NatMemory");

        Instance->m_smallAllocator->TrimBlocks();
        Instance->m_largeAllocator->TrimBlocks();
    }
}
void RuntimeManager::LateUpdate()
{
    PROFILESTACK("Runtime Late Update");

    mono_runtime_invoke(Instance->m_lateUpdateMethod, NULL, NULL, NULL);
}

void RuntimeManager::BindFunction(const char* a_location, void* a_function)
{
    mono_add_internal_call(a_location, a_function);
}
void RuntimeManager::BindFunction(const IcarianCore::COWU8String& a_location, void* a_function)
{
    const char* str = a_location.CStr();
    BindFunction(str, a_function);
}

void RuntimeManager::AttachThread()
{
    mono_jit_thread_attach(Instance->m_domain);
}

void RuntimeManager::PushDLLPath(const char* a_path)
{
    const IcarianCore::COWU8String path = IcarianCore::COWU8String(a_path, Alloc);

    PushDLLPath(path);
}
void RuntimeManager::PushDLLPath(const IcarianCore::COWU8String& a_path)
{
    const IcarianCore::COWU8String ext = IO::GetExtension(a_path, Alloc);
    if (!ext.Empty())
    {
        switch (StringHash(ext.CStr()))
        {
        case StringHash(".dll"):
        case StringHash(".so"):
        {
            break;
        }
        default:
        {
            IERROR("Invalid DLL type");

            break;
        }
        }
    }

    const IcarianCore::COWU8String filename = IO::GetFilename(a_path, Alloc);
    const IcarianCore::COWU8String path = IcarianCore::COWU8String(a_path, Alloc);

    Instance->m_dllLookup->Push(filename, path);
}
IcarianCore::COWU8String RuntimeManager::GetDLLPath(const char* a_path)
{
    if (Instance == nullptr)
    {
        return IcarianCore::COWU8String(Alloc);
    }

    const IcarianCore::COWU8String path = IcarianCore::COWU8String(a_path, Alloc);

    return GetDLLPath(path);
}
IcarianCore::COWU8String RuntimeManager::GetDLLPath(const IcarianCore::COWU8String& a_path)
{
    if (Instance == nullptr)
    {
        return IcarianCore::COWU8String(Alloc);
    }

    const IcarianCore::COWU8String filename = IO::GetFilename(a_path, Alloc);
    if (Instance->m_dllLookup->Exists(filename))
    {
        return Instance->m_dllLookup->GetValue(filename);
    }

    const IcarianCore::COWU8String libFilename = "lib" + filename;
    if (Instance->m_dllLookup->Exists(libFilename))
    {
        return Instance->m_dllLookup->GetValue(libFilename);
    }

    return IcarianCore::COWU8String(Alloc);
}

MonoDomain* RuntimeManager::GetDomain()
{
    return Instance->m_domain;
}

MonoClass* RuntimeManager::GetClass(const char* a_namespace, const char* a_name)
{
    return mono_class_from_name(Instance->m_image, a_namespace, a_name);
}
MonoClass* RuntimeManager::GetClass(const IcarianCore::COWU8String& a_namespace, const IcarianCore::COWU8String& a_name)
{
    const char* ns = a_namespace.CStr();
    const char* n = a_name.CStr();

    return GetClass(ns, n);
}

RuntimeFunction* RuntimeManager::GetFunction(const char* a_namespace, const char* a_class, const char* a_method)
{
    MonoClass* cls = mono_class_from_name(Instance->m_image, a_namespace, a_class);
    IVERIFY(cls != NULL);

    MonoMethodDesc* desc = mono_method_desc_new(a_method, 0);
    IVERIFY(desc != NULL);
    IDEFER(mono_method_desc_free(desc));
    MonoMethod* method = mono_method_desc_search_in_class(desc, cls);
    IVERIFY(method != NULL);

    return IcarianCore::MallocAllocator::Instance->Create<RuntimeFunction>(method);
}
RuntimeFunction* RuntimeManager::GetFunction(const IcarianCore::COWU8String& a_namespace, const IcarianCore::COWU8String& a_class, const IcarianCore::COWU8String& a_method)
{
    const char* ns = a_namespace.CStr();
    const char* c = a_class.CStr();
    const char* m = a_method.CStr();

    return GetFunction(ns, c, m);
}

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
