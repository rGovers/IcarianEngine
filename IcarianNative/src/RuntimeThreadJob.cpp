#include "RuntimeThreadJob.h"

#include "ThreadPool.h"

RuntimeThreadJob::RuntimeThreadJob(uint32_t a_objectAddr, e_JobPriority a_priority) : ThreadJob(a_priority)
{
    m_objectAddr = a_objectAddr;
}
RuntimeThreadJob::~RuntimeThreadJob()
{

}

void RuntimeThreadJob::Execute()
{
    ThreadPool::Dispath(m_objectAddr);
}