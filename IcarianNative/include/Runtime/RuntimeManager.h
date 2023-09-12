#pragma once

#include <cstdint>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <string_view>

class RenderEngine;
class RuntimeFunction;

#define FLARE_MONO_EXPORT(ret, func, ...) static ret func(__VA_ARGS__)

#define RUNTIME_FUNCTION_NAME(klass, name) MRF_##klass##_##name
#define RUNTIME_FUNCTION_STRING(namespace, klass, name) #namespace "." #klass "::" #name

#define RUNTIME_FUNCTION(ret, klass, name, code, ...) FLARE_MONO_EXPORT(ret, RUNTIME_FUNCTION_NAME(klass, name), __VA_ARGS__) code

#define RUNTIME_FUNCTION_DEFINITION(ret, namespace, klass, name, code, ...) RUNTIME_FUNCTION(ret, klass, name, code, __VA_ARGS__)

#define BIND_FUNCTION(runtime, namespace, klass, name) runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name))

class RuntimeManager
{
private:
    MonoDomain*   m_domain;
    MonoAssembly* m_assembly;

    MonoImage*    m_image;

    MonoClass*    m_programClass;

    MonoMethod*   m_updateMethod;
    MonoMethod*   m_shutdownMethod;

protected:

public:
    RuntimeManager();
    ~RuntimeManager();

    void BindFunction(const std::string_view& a_location, void* a_function);

    void Exec(int32_t a_argc, char* a_argv[]);
    void Update(double a_delta, double a_time);

    void AttachThread();

    inline MonoDomain* GetDomain() const
    {
        return m_domain;
    }

    MonoClass* GetClass(const std::string_view& a_namespace, const std::string_view& a_name) const;

    RuntimeFunction* GetFunction(const std::string_view& a_namespace, const std::string_view& a_class, const std::string_view& a_method) const;
};