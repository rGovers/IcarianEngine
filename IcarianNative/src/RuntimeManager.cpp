#include "Runtime/RuntimeManager.h"

#include <cstring>
#include <filesystem>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>
#include <mono/utils/mono-dl-fallback.h>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Runtime/RuntimeFunction.h"

static RuntimeManager* Instance = nullptr;

#include "EngineIcarianAssemblyInterop.h"

ENGINE_ICARIANASSEMBLY_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

#ifndef WIN32
#include <dlfcn.h>

#include "Flare/MonoNativeImpl.h"

static constexpr char MonoNativeLibName[] = "libmono-native.so";
static constexpr uint32_t MonoNativeLibNameLength = sizeof(MonoNativeLibName) - 1;
// Ludum Dare 54 hack 
static constexpr char MonoNativeBaseName[] = "System.Native";
static constexpr uint32_t MonoNativeBaseNameLength = sizeof(MonoNativeBaseName) - 1;

#define MonoThisLibHandle ((void*)-1)
#else
#include "Flare/WindowsHeaders.h"
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
            ICARIAN_ASSERT_MSG(handle != NULL, std::string("Failed to open DLL: ") + dlerror());

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
        return FlareBase::MonoNativeImpl::GetFunction(a_name);
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
    FlareBase::MonoNativeImpl::Init();
#endif

    mono_dl_fallback_register(RuntimeDLOpen, RuntimeDLSymbol, NULL, NULL);

    m_domain = mono_jit_init_version("Core", "v4.0");
    m_assembly = mono_domain_assembly_open(m_domain, "IcarianCS.dll");
    ICARIAN_ASSERT(m_assembly != NULL);

    m_image = mono_assembly_get_image(m_assembly);
    ICARIAN_ASSERT(m_image != NULL);
    m_programClass = mono_class_from_name(m_image, "IcarianEngine", "Program");
    ICARIAN_ASSERT(m_programClass != NULL);

    MonoMethodDesc* updateDesc = mono_method_desc_new(":Update(double,double)", 0);
    m_updateMethod = mono_method_desc_search_in_class(updateDesc, m_programClass);
    IDEFER(mono_method_desc_free(updateDesc));
    ICARIAN_ASSERT(m_updateMethod != NULL);

    MonoMethodDesc* shutdownDesc = mono_method_desc_new(":Shutdown()", 0);
    m_shutdownMethod = mono_method_desc_search_in_class(shutdownDesc, m_programClass);
    IDEFER(mono_method_desc_free(shutdownDesc));
    ICARIAN_ASSERT(m_shutdownMethod != NULL);

    ENGINE_ICARIANASSEMBLY_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
RuntimeManager::~RuntimeManager()
{
    mono_runtime_invoke(m_shutdownMethod, NULL, NULL, NULL);

    mono_free_method(m_updateMethod);
    mono_free_method(m_shutdownMethod);

    mono_jit_cleanup(m_domain);

#ifndef WIN32
    mono_dl_fallback_unregister(NULL);

    FlareBase::MonoNativeImpl::Destroy();
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
    const int retVal = mono_jit_exec(Instance->m_domain, Instance->m_assembly, a_argc, a_argv);
    ICARIAN_ASSERT(retVal == 0);
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
    ICARIAN_ASSERT(cls != NULL);

    MonoMethodDesc* desc = mono_method_desc_new(a_method.data(), 0);
    IDEFER(mono_method_desc_free(desc));
    MonoMethod* method = mono_method_desc_search_in_class(desc, cls);
    ICARIAN_ASSERT(method != NULL);

    return new RuntimeFunction(method);
}