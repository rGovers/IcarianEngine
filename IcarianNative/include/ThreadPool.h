#pragma once

#include <queue>

#include "ThreadJob.h"

class RuntimeFunction;
class RuntimeManager;

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

    RuntimeManager*                                                 m_runtime;
    RuntimeFunction*                                                m_runtimeDispatch;

    std::thread*                                                    m_threads;

    volatile bool*                                                  m_join;
    volatile bool                                                   m_shutdown;

    std::mutex                                                      m_lock;
    std::condition_variable                                         m_jobAvailable;

    std::priority_queue<ThreadJob*, std::vector<ThreadJob*>, JLess> m_jobQueue;

    static void Run(uint32_t a_thread);

    ThreadPool(uint32_t a_threadCount, RuntimeManager* a_runtime);

    void Start();

protected:

public:
    ~ThreadPool();

    static void Init(RuntimeManager* a_runtime);
    static void Stop();
    static void Destroy();

    static uint32_t GetThreadCount();

    static void PushJob(ThreadJob* a_job);

    static void Dispath(uint32_t a_objectAddr);
};