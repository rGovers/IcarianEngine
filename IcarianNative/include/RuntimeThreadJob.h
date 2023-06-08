#pragma once

#include "ThreadJob.h"

#include "Runtime/RuntimeManager.h"

class RuntimeThreadJob : public ThreadJob
{
private:
    uint32_t        m_objectAddr;

protected:

public:
    RuntimeThreadJob(uint32_t a_objectAddr, e_JobPriority a_priority);
    virtual ~RuntimeThreadJob();

    virtual void Execute();
};