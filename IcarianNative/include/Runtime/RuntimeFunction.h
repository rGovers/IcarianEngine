#pragma once

#include <mono/jit/jit.h>

class RuntimeFunction
{
private:
    MonoMethod* m_method;

protected:

public:
    RuntimeFunction(MonoMethod* a_method);
    ~RuntimeFunction();

    void Exec(void** a_params);
};