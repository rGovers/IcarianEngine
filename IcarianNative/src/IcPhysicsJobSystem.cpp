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