// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <Jolt/Jolt.h>

#include <cstdint>
#include <Jolt/Core/Color.h>
#include <Jolt/Core/FixedSizeFreeList.h>
#include <Jolt/Core/JobSystemWithBarrier.h>

#include "ThreadPool.h"

class IcPhysicsJobSystem : public JPH::JobSystemWithBarrier
{
private:
    static constexpr uint32_t MaxJobs = 1024;

    JPH::FixedSizeFreeList<Job> m_jobs;

    class PhysicsJob : public ThreadJob
    {
    private:
        Job* m_job;

    protected:

    public:
        PhysicsJob(Job* a_job) : ThreadJob(JobPriority_EngineHigh)
        {
            m_job = a_job;

            m_job->AddRef();
        }
        virtual ~PhysicsJob()
        {
            m_job->Release();
        }

        virtual void Execute()
        {
            m_job->Execute();
        }
    };

protected:
    virtual void QueueJob(Job* a_job);
    virtual void QueueJobs(Job** a_jobs, JPH::uint a_numJobs);
    virtual void FreeJob(Job* a_job);

public:
    IcPhysicsJobSystem(JPH::uint a_numBarriers);
    virtual ~IcPhysicsJobSystem();

    virtual int GetMaxConcurrency() const;

    virtual JobHandle CreateJob(const char* a_name, JPH::ColorArg a_color, const JobFunction& a_jobFunction, uint32_t a_numDependencies);
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