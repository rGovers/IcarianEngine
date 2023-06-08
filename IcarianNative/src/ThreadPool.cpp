#include "ThreadPool.h"

#include <chrono>

#include "Runtime/RuntimeManager.h"
#include "Runtime/RuntimeFunction.h"
#include "RuntimeThreadJob.h"
#include "Trace.h"

static ThreadPool* Instance = nullptr;

RUNTIME_FUNCTION(void, ThreadPool, AddJob, 
{
    RuntimeThreadJob* job = new RuntimeThreadJob(a_objectAddr, (e_JobPriority)a_priority);

    ThreadPool::PushJob(job);
}, uint32_t a_objectAddr, uint32_t a_priority)

ThreadPool::ThreadPool(uint32_t a_threadCount, RuntimeManager* a_runtime)
{
    m_runtime = a_runtime;

    m_shutdown = false;

    m_threadCount = a_threadCount;

    m_threads = new std::thread[m_threadCount];
    m_join = new bool[m_threadCount];

    m_runtimeDispatch = m_runtime->GetFunction("IcarianEngine", "ThreadPool", ":Dispatch(uint)");

    for (uint32_t i = 0; i < m_threadCount; ++i)
    {
        m_join[i] = false;
    }
}
ThreadPool::~ThreadPool()
{
    m_shutdown = true;
    m_jobAvailable.notify_all();

    for (uint32_t i = 0; i < m_threadCount; ++i)
    {
        while (!m_join[i]) 
        {
            std::this_thread::yield();
        }

        m_threads[i].join();
    }

    delete[] m_threads;
    delete[] m_join;

    while (!m_jobQueue.empty())
    {
        ThreadJob* job = m_jobQueue.top();
        m_jobQueue.pop();

        delete job;
    }   

    delete m_runtimeDispatch;
}

void ThreadPool::Start()
{
    for (uint32_t i = 0; i < m_threadCount; ++i)
    {
        m_threads[i] = std::thread(ThreadPool::Run, i);
    }
}
void ThreadPool::Stop()
{
    if (Instance != nullptr)
    {
        TRACE("Stopping thread pool");

        Instance->m_shutdown = true;
        Instance->m_jobAvailable.notify_all();
    }
}

void ThreadPool::Init(RuntimeManager* a_runtime)
{
    if (Instance == nullptr)
    {
        TRACE("Starting thread pool");

        Instance = new ThreadPool((uint32_t)std::thread::hardware_concurrency() / 2, a_runtime);
        // Instance = new ThreadPool(2, a_runtime);
        Instance->Start();

        BIND_FUNCTION(a_runtime, IcarianEngine, ThreadPool, AddJob);
    }
}
void ThreadPool::Destroy()
{
    if (Instance != nullptr)
    {
        TRACE("Destroying thread pool");

        delete Instance;
        Instance = nullptr;
    }
}

uint32_t ThreadPool::GetThreadCount()
{
    return Instance->m_threadCount;
}

void ThreadPool::PushJob(ThreadJob* a_job)
{
    {
        const std::lock_guard l = std::lock_guard(Instance->m_lock);

        Instance->m_jobQueue.push(a_job);
    }
    
    Instance->m_jobAvailable.notify_one();
}

void ThreadPool::Run(uint32_t a_thread)
{
    Instance->m_runtime->AttachThread();

    while (!Instance->m_shutdown) 
    {
        ThreadJob* job = nullptr;
        
        {
            std::unique_lock l = std::unique_lock(Instance->m_lock);
            Instance->m_jobAvailable.wait(l, [] { return !Instance->m_jobQueue.empty() || Instance->m_shutdown; });
            
            if (!Instance->m_jobQueue.empty())
            {
                job = Instance->m_jobQueue.top();

                Instance->m_jobQueue.pop();
            }
        }

        if (job != nullptr)
        {
            job->Execute();

            delete job;
        }
    }

    Instance->m_join[a_thread] = true;
}

void ThreadPool::Dispath(uint32_t a_objectAddr)
{
    void* args[] =
    {
        &a_objectAddr
    };

    Instance->m_runtimeDispatch->Exec(args);
}