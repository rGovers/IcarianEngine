#include "DeletionQueue.h"

#include "Core/IcarianDefer.h"

static DeletionQueue* Instance = nullptr;

DeletionQueue::DeletionQueue()
{
    for (uint32_t i = 0; i < DeletionIndex_Last; i++)
    {
        m_queueIndex[i] = 0;
    }
}
DeletionQueue::~DeletionQueue()
{
    for (uint32_t i = 0; i < DeletionIndex_Last; i++)
    {
        for (uint32_t j = 0; j < QueueSize; j++)
        {
            for (uint32_t k = 0; k < m_deletionObjects[j][i].Size(); k++)
            {
                DeletionObject* object = m_deletionObjects[j][i][k];
                if (object != nullptr)
                {
                    object->Destroy();

                    delete object;
                }
            }
        }
    }
}

void DeletionQueue::Init()
{
    if (Instance == nullptr)
    {
        Instance = new DeletionQueue();
    }
}
void DeletionQueue::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void DeletionQueue::Push(DeletionObject* a_object, e_DeletionIndex a_index)
{
    const uint32_t index = Instance->m_queueIndex[a_index];

    Instance->m_deletionObjects[index][a_index].Push(a_object);
}

void DeletionQueue::Flush(e_DeletionIndex a_index)
{
    const uint32_t index = Instance->m_queueIndex[a_index];
    const uint32_t nextIndex = (index + 1) % QueueSize;
    IDEFER(Instance->m_queueIndex[a_index] = nextIndex);
    
    const TLockArray<DeletionObject*> a = Instance->m_deletionObjects[nextIndex][a_index].ToLockArray();
    for (DeletionObject* object : a)
    {
        if (object == nullptr)
        {
            continue;
        }

        object->Destroy();

        delete object;
    }

    Instance->m_deletionObjects[nextIndex][a_index].UClear();
}
void DeletionQueue::ClearQueue(e_DeletionIndex a_index)
{
    for (uint32_t i = 0; i < QueueSize; ++i)
    {
        Flush(a_index);
    }
}