#pragma once

#include <condition_variable>
#include <queue>

#include "DataTypes/TArray.h"
#include "ThreadJob.h"

class RuntimeFunction;

class ThreadPool
{
private:
    struct JLess
    {
        bool operator()(const ThreadJob* a_lhs, const ThreadJob* a_rhs) const
        {
            return a_rhs->GetPriority() > a_lhs->GetPriority();
        }
    };

    uint32_t                                                        m_threadCount;

    RuntimeFunction*                                                m_runtimeDispatch;

    std::thread*                                                    m_threads;

    volatile bool*                                                  m_join;
    volatile bool                                                   m_shutdown;

    std::mutex                                                      m_lock;
    std::condition_variable                                         m_jobAvailable;

    TArray<std::shared_mutex*>                                      m_runtimeLocks;                                

    std::priority_queue<ThreadJob*, std::vector<ThreadJob*>, JLess> m_jobQueue;

    static void Run(uint32_t a_thread);

    void Start();

    ThreadPool(uint32_t a_threadCount);

protected:

public:
    ~ThreadPool();

    static void Init();
    static void Stop();
    static void Destroy();

    static uint32_t GenerateLock();
    static void DestroyLock(uint32_t a_addr);
    static void ReadLock(uint32_t a_addr);
    static void ReadUnlock(uint32_t a_addr);
    static void WriteLock(uint32_t a_addr);
    static void WriteUnlock(uint32_t a_addr);

    static uint32_t GetThreadCount();
    static uint32_t GetQueueSize();

    static void PushJob(ThreadJob* a_job);

    static void Dispath(uint32_t a_objectAddr);
};