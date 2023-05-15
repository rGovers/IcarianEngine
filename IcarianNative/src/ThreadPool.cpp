#include "ThreadPool.h"

#include <chrono>

#include "Trace.h"

static ThreadPool* Instance = nullptr;

ThreadPool::ThreadPool(uint32_t a_threadCount)
{
    m_shutdown = false;

    m_threadCount = a_threadCount;

    m_threads = new std::thread[m_threadCount];
    m_join = new bool[m_threadCount];

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
}

void ThreadPool::Start()
{
    for (uint32_t i = 0; i < m_threadCount; ++i)
    {
        m_threads[i] = std::thread(ThreadPool::Run, i);
    }
}

void ThreadPool::Init()
{
    if (Instance == nullptr)
    {
        TRACE("Starting thread pool");

        Instance = new ThreadPool((uint32_t)std::thread::hardware_concurrency() / 2);
        Instance->Start();
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