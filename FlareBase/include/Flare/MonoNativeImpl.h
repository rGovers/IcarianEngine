#pragma once

#include <string>
#include <unordered_map>

namespace FlareBase
{   
    class MonoNativeImpl
    {
    private:
        std::unordered_map<std::string, void*> m_functions;
    
        MonoNativeImpl();
    
    protected:
    
    public:
        ~MonoNativeImpl();
    
        static void Init();
        static void Destroy();
    
        static void* GetFunction(const std::string_view& a_name);
    };
}
