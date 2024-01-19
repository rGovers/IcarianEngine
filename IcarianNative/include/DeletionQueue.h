#pragma once

#include "DataTypes/TArray.h"

enum e_DeletionIndex
{
    DeletionIndex_Update = 0,
    DeletionIndex_Render = 1,
    DeletionIndex_Last
};

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
private:
    constexpr static uint32_t QueueSize = 2;

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