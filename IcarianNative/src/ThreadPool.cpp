#include "ThreadPool.h"

#include <chrono>

#include "Flare/IcarianAssert.h"
#include "Flare/IcarianDefer.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"
#include "Runtime/RuntimeFunction.h"
#include "RuntimeThreadJob.h"
#include "Trace.h"

static ThreadPool* Instance = nullptr;

#define THREADPOOL_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) BIND_FUNCTION(a_runtime, namespace, klass, name);

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define THREADPOOL_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine, ThreadPool, AddJob, { ThreadPool::PushJob(new RuntimeThreadJob(a_objectAddr, (e_JobPriority)a_priority)); }, uint32_t a_objectAddr, uint32_t a_priority) \
    F(uint32_t, IcarianEngine, ThreadPool, GetThreadCount, { return ThreadPool::GetThreadCount(); }) \
    \
    F(uint32_t, IcarianEngine, NativeLock, GenerateLock, { return ThreadPool::GenerateLock(); }) \
    F(void, IcarianEngine, NativeLock, DestroyLock, { ThreadPool::DestroyLock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SReadLock, { ThreadPool::ReadLock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SReadUnlock, { ThreadPool::ReadUnlock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SWriteLock, { ThreadPool::WriteLock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SWriteUnlock, { ThreadPool::WriteUnlock(a_addr); }, uint32_t a_addr) \

THREADPOOL_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

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

    for (uint32_t i = 0; i < m_runtimeLocks.Size(); ++i)
    {
        if (m_runtimeLocks[i] != nullptr)
        {
            Logger::Warning("Lock was not destroyed");

            delete m_runtimeLocks[i];
        }
    }

    m_runtimeLocks.Clear();
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

        // Should give me a lot of threads without overallocating unless the system is really bad (<= 4 threads) and/or Mono says fuck you JIT/GC time
        Instance = new ThreadPool((uint32_t)std::thread::hardware_concurrency() / 2, a_runtime);
        // Instance = new ThreadPool(2, a_runtime);
        Instance->Start();

        THREADPOOL_BINDING_FUNCTION_TABLE(THREADPOOL_RUNTIME_ATTACH);
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

uint32_t ThreadPool::GenerateLock()
{
    std::shared_mutex* mutex = new std::shared_mutex();

    {
        TLockArray<std::shared_mutex*> a = Instance->m_runtimeLocks.ToLockArray();
        const uint32_t count = a.Size();

        for (uint32_t i = 0; i < count; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = mutex;

                return i;
            }
        }
    }

    return Instance->m_runtimeLocks.PushVal(mutex);
}
void ThreadPool::DestroyLock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "DestroyLock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "DetroyLock lock is already destroyed");

    const std::shared_mutex* mutex = Instance->m_runtimeLocks[a_addr];
    ICARIAN_DEFER_del(mutex);
    Instance->m_runtimeLocks.LockSet(a_addr, nullptr);
}

void ThreadPool::ReadLock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "ReadLock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "ReadLock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->lock_shared();
}
void ThreadPool::ReadUnlock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "ReadUnlock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "ReadUnlock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->unlock_shared();
}
void ThreadPool::WriteLock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "WriteLock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "WriteLock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->lock();
}
void ThreadPool::WriteUnlock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "WriteUnlock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "WriteUnlock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->unlock();
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
        ThreadJob** jobPtr = &job;
        ICARIAN_DEFER(jobPtr, if (*jobPtr != nullptr) { delete *jobPtr; });

        {
            std::unique_lock l = std::unique_lock(Instance->m_lock);
            Instance->m_jobAvailable.wait(l, []() { return Instance->m_shutdown || !Instance->m_jobQueue.empty(); });

            if (Instance->m_shutdown)
            {
                break;
            }

            job = Instance->m_jobQueue.top();

            Instance->m_jobQueue.pop();
        }

        if (job != nullptr)
        {
            job->Execute();
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