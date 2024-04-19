#pragma once

#include "DataTypes/TArray.h"

enum e_DeletionIndex
{
    DeletionIndex_Update = 0,
    DeletionIndex_Render = 1,
    DeletionIndex_Last
};

// Allow for defering deletion to the end of a threads loop in a thread safe manner
// Not to be confused with Vulkans Deletion Queue which is based off flight frames
// Preferred to use for C# binding code
// TODO: Need to migrate the rest of the binding code to use it
class DeletionObject
{
private:

protected:

public:
    virtual ~DeletionObject() { }
    virtual void Destroy() = 0;
};

class DeletionQueue
{
public:
    constexpr static uint32_t QueueSize = 2;
    
private:

    uint32_t                m_queueIndex[DeletionIndex_Last];
    TArray<DeletionObject*> m_deletionObjects[QueueSize][DeletionIndex_Last];

    DeletionQueue();

protected:

public:
    ~DeletionQueue();

    static void Init();
    static void Destroy();

    static void Push(DeletionObject* a_object, e_DeletionIndex a_index);

    static void Flush(e_DeletionIndex a_index);
};

#define ICARIAN_DELFUNC_NAMEI(a, b) a##b
#define ICARIAN_DELFUNC_NAMEM(a, b) ICARIAN_DELFUNC_NAMEI(a, b)
#define ICARIAN_DELFUNC_NAME(a) ICARIAN_DELFUNC_NAMEM(a, __LINE__)

// Not the greatest as it is just wrapping a lambda and comes with overhead but cleans up boiler plate code
// Have not checked if the compiler inlines the call it may not as it crosses scope on the heap and relying on the vtable
// If lucky gets inlined otherwise get an addition function call to execute the lambda
// Anyway pretty sure just gave the "C++" developers an aneuyrsm for using macros rather then just templating it
// Repeat after me templates are for data not functionality
#define IPUSHDELETIONFUNC(code, deletionIndex) \
    const auto ICARIAN_DELFUNC_NAME(_delObj) = [=] { code; }; \
    using ICARIAN_DELFUNC_NAME(_t) = decltype(ICARIAN_DELFUNC_NAME(_delObj)); \
    class ICARIAN_DELFUNC_NAME(_delObj_class) : public DeletionObject \
    { \
    private: \
        ICARIAN_DELFUNC_NAME(_t) m_val; \
    protected: \
    public: \
        ICARIAN_DELFUNC_NAME(_delObj_class)(ICARIAN_DELFUNC_NAME(_t) a_val) : m_val(a_val) { } \
        virtual void Destroy() \
        { \
            m_val(); \
        } \
    }; \
    DeletionQueue::Push(new ICARIAN_DELFUNC_NAME(_delObj_class)(ICARIAN_DELFUNC_NAME(_delObj)), deletionIndex)
