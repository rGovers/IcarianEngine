// Icarian Engine - C# Game Engine
// 
// License at end of file.

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

    RuntimeFunction*                                                m_runtimeDispatch;

    std::thread*                                                    m_threads;

    std::mutex                                                      m_lock;
    std::condition_variable                                         m_jobAvailable;

    TArray<SharedSpinLock*>                                         m_runtimeLocks;                                

    std::priority_queue<ThreadJob*, std::vector<ThreadJob*>, JLess> m_jobQueue;

    volatile bool*                                                  m_join;

    uint32_t                                                        m_threadCount;
    volatile bool                                                   m_shutdown;

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