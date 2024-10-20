// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "ThreadPool.h"

#include <chrono>
#include <glm/glm.hpp>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "Logger.h"
#include "Runtime/RuntimeManager.h"
#include "Runtime/RuntimeFunction.h"
#include "RuntimeThreadJob.h"
#include "Trace.h"

static ThreadPool* Instance = nullptr;

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define THREADPOOL_BINDING_FUNCTION_TABLE(F) \
    F(void, IcarianEngine, ThreadPool, AddJob, { ThreadPool::PushJob(new RuntimeThreadJob(a_objectAddr, (e_JobPriority)a_priority)); }, uint32_t a_objectAddr, uint32_t a_priority) \
    F(uint32_t, IcarianEngine, ThreadPool, GetThreadCount, { return ThreadPool::GetThreadCount(); }) \
    F(uint32_t, IcarianEngine, ThreadPool, GetQueueSize, { return ThreadPool::GetQueueSize(); }) \
    \
    F(uint32_t, IcarianEngine, NativeLock, GenerateLock, { return ThreadPool::GenerateLock(); }) \
    F(void, IcarianEngine, NativeLock, DestroyLock, { ThreadPool::DestroyLock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SReadLock, { ThreadPool::ReadLock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SReadUnlock, { ThreadPool::ReadUnlock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SWriteLock, { ThreadPool::WriteLock(a_addr); }, uint32_t a_addr) \
    F(void, IcarianEngine, NativeLock, SWriteUnlock, { ThreadPool::WriteUnlock(a_addr); }, uint32_t a_addr) \

THREADPOOL_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

ThreadPool::ThreadPool(uint32_t a_threadCount)
{
    m_shutdown = false;

    m_threadCount = a_threadCount;

    m_threads = new std::thread[m_threadCount];
    m_join = new bool[m_threadCount];

    m_runtimeDispatch = RuntimeManager::GetFunction("IcarianEngine", "ThreadPool", ":Dispatch(uint)");

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

void ThreadPool::Init()
{
    if (Instance == nullptr)
    {
        TRACE("Starting thread pool");

        // Should give me a lot of threads without overallocating unless the system is really bad (<= 4 threads) and/or Mono says fuck you JIT/GC time
        Instance = new ThreadPool(glm::max((uint32_t)std::thread::hardware_concurrency() / 2, 2U));
        // Instance = new ThreadPool(2);
        Instance->Start();

        THREADPOOL_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_ATTACH);
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
    SharedSpinLock* lock = new SharedSpinLock();

    {
        TLockArray<SharedSpinLock*> a = Instance->m_runtimeLocks.ToLockArray();
        const uint32_t count = a.Size();

        for (uint32_t i = 0; i < count; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = lock;

                return i;
            }
        }
    }

    return Instance->m_runtimeLocks.PushVal(lock);
}
void ThreadPool::DestroyLock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "DestroyLock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "DetroyLock lock is already destroyed");

    const SharedSpinLock* lock = Instance->m_runtimeLocks[a_addr];
    IDEFER(delete lock);
    Instance->m_runtimeLocks.LockSet(a_addr, nullptr);
}

void ThreadPool::ReadLock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "ReadLock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "ReadLock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->LockShared();
}
void ThreadPool::ReadUnlock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "ReadUnlock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "ReadUnlock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->UnlockShared();
}
void ThreadPool::WriteLock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "WriteLock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "WriteLock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->Lock();
}
void ThreadPool::WriteUnlock(uint32_t a_addr)
{
    ICARIAN_ASSERT_MSG(a_addr < Instance->m_runtimeLocks.Size(), "WriteUnlock invalid lock address");
    ICARIAN_ASSERT_MSG(Instance->m_runtimeLocks[a_addr] != nullptr, "WriteUnlock lock is already destroyed");

    Instance->m_runtimeLocks[a_addr]->Unlock();
}

uint32_t ThreadPool::GetThreadCount()
{
    return Instance->m_threadCount;
}
uint32_t ThreadPool::GetQueueSize()
{
    return (uint32_t)Instance->m_jobQueue.size();
}

void ThreadPool::PushJob(ThreadJob* a_job)
{
    {
        const std::unique_lock l = std::unique_lock(Instance->m_lock);

        Instance->m_jobQueue.push(a_job);
    }
    
    Instance->m_jobAvailable.notify_one();
}

void ThreadPool::Run(uint32_t a_thread)
{
    RuntimeManager::AttachThread();

    while (!Instance->m_shutdown) 
    {
        ThreadJob* job = nullptr;
        ThreadJob** jobPtr = &job;
        IDEFER(
        if (*jobPtr != nullptr) 
        {
            delete *jobPtr; 
        });

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

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.