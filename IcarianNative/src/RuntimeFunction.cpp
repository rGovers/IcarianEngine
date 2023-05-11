#include "Runtime/RuntimeFunction.h"

RuntimeFunction::RuntimeFunction(MonoMethod* a_method)
{
    m_method = a_method;
}
RuntimeFunction::~RuntimeFunction()
{
    mono_free_method(m_method);
}

void RuntimeFunction::Exec(void** a_params)
{
    mono_runtime_invoke(m_method, nullptr, a_params, nullptr);
}