#include "Runtime/RuntimeManager.h"

#include <assert.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>

#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Runtime/RuntimeFunction.h"

RuntimeManager::RuntimeManager()
{
    mono_config_parse(NULL);

    mono_set_dirs("./lib", "./etc");
    
    m_domain = mono_jit_init_version("Core", "v4.0");
    m_assembly = mono_domain_assembly_open(m_domain, "FlareCS.dll");
    assert(m_assembly != nullptr);

    m_image = mono_assembly_get_image(m_assembly);
    assert(m_image != nullptr);
    m_programClass = mono_class_from_name(m_image, "IcarianEngine", "Program");
    assert(m_programClass != nullptr);

    MonoMethodDesc* updateDesc = mono_method_desc_new(":Update(double,double)", 0);
    m_updateMethod = mono_method_desc_search_in_class(updateDesc, m_programClass);
    assert(m_updateMethod != nullptr);

    MonoMethodDesc* shutdownDesc = mono_method_desc_new(":Shutdown()", 0);
    m_shutdownMethod = mono_method_desc_search_in_class(shutdownDesc, m_programClass);
    assert(m_shutdownMethod != nullptr);

    mono_method_desc_free(updateDesc);
    mono_method_desc_free(shutdownDesc);
}
RuntimeManager::~RuntimeManager()
{
    mono_runtime_invoke(m_shutdownMethod, nullptr, nullptr, nullptr);

    mono_free_method(m_updateMethod);
    mono_free_method(m_shutdownMethod);

    mono_jit_cleanup(m_domain);

    // TODO: Find non-locking mono cleanup
}

void RuntimeManager::Exec(int a_argc, char* a_argv[])
{
    const int retVal = mono_jit_exec(m_domain, m_assembly, a_argc, a_argv);
    assert(retVal == 0);
}
void RuntimeManager::Update(double a_delta, double a_time)
{
    PROFILESTACK("Runtime Update");
    
    void* args[2];
    args[0] = &a_delta;
    args[1] = &a_time;

    mono_runtime_invoke(m_updateMethod, nullptr, args, nullptr);
}

void RuntimeManager::BindFunction(const std::string_view& a_location, void* a_function)
{
    mono_add_internal_call(a_location.data(), a_function);
}

void RuntimeManager::AttachThread()
{
    mono_jit_thread_attach(m_domain);
}

MonoClass* RuntimeManager::GetClass(const std::string_view& a_namespace, const std::string_view& a_name) const
{
    return mono_class_from_name(m_image, a_namespace.data(), a_name.data());
}

RuntimeFunction* RuntimeManager::GetFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method) const
{
    MonoClass* cls = mono_class_from_name(m_image, a_namespace.data(), a_class.data());
    assert(cls != nullptr);

    MonoMethodDesc* desc = mono_method_desc_new(a_method.data(), 0);
    MonoMethod* method = mono_method_desc_search_in_class(desc, cls);
    assert(method != nullptr);

    RuntimeFunction* func = new RuntimeFunction(method);

    mono_method_desc_free(desc);

    return func;
}