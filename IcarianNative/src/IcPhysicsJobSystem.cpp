// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Physics/IcPhysicsJobSystem.h"

IcPhysicsJobSystem::IcPhysicsJobSystem(JPH::uint a_numBarriers)
{
    Init(a_numBarriers);

    m_jobs.Init(MaxJobs, MaxJobs >> 3);
}
IcPhysicsJobSystem::~IcPhysicsJobSystem()
{
    
}

int IcPhysicsJobSystem::GetMaxConcurrency() const
{
    return (int)ThreadPool::GetThreadCount();
}

JPH::JobHandle IcPhysicsJobSystem::CreateJob(const char* a_name, JPH::ColorArg a_color, const JobFunction& a_jobFunction, uint32_t a_numDependencies)
{
    JPH::uint32 index = JPH::FixedSizeFreeList<Job>::cInvalidObjectIndex; 

    do 
    {
        index = m_jobs.ConstructObject(a_name, a_color, this, a_jobFunction, a_numDependencies);
    }
    while (index == JPH::FixedSizeFreeList<Job>::cInvalidObjectIndex);

    Job* job = &m_jobs.Get(index);

    const JobHandle handle = JobHandle(job);

    if (a_numDependencies == 0)
    {
        QueueJob(job);
    }

    return handle;
}
void IcPhysicsJobSystem::FreeJob(Job* a_job)
{
    m_jobs.DestructObject(a_job);
}

void IcPhysicsJobSystem::QueueJob(Job* a_job)
{
    ThreadPool::PushJob(new PhysicsJob(a_job));
}
void IcPhysicsJobSystem::QueueJobs(Job** a_jobs, JPH::uint a_numJobs)
{
    for (JPH::uint i = 0; i < a_numJobs; ++i)
    {        
        ThreadPool::PushJob(new PhysicsJob(a_jobs[i]));
    }
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