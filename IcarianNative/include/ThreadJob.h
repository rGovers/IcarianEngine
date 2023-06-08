#pragma once

#include <future>

// Want the engine to take precedence over the runtime
enum e_JobPriority : uint32_t
{
    // Reserved for render jobs so that the frame can get out ASAP
    // Otherwise I can hear the screaming about the completely unplayable 10000fps already
    JobPriority_EngineUrgent = 6,
    JobPriority_EngineHigh = 5,
    JobPriority_EngineMedium = 3,
    JobPriority_EngineLow = 1,

    JobPriority_RuntimeHigh = 4,
    JobPriority_RuntimeMedium = 2,
    JobPriority_RuntimeLow = 0
};

class ThreadJob
{
private:
    e_JobPriority m_priority;

protected:

public:
    constexpr ThreadJob(e_JobPriority a_priority) :
        m_priority(a_priority)
    {

    }
    virtual ~ThreadJob() {}

    constexpr e_JobPriority GetPriority() const
    {
        return m_priority;
    }

    virtual void Execute() = 0;
};

template<typename R, class F>
class FThreadJob : public ThreadJob
{
private:
    std::promise<R> m_promise;
    F               m_func;

protected:

public:
    FThreadJob(F a_func, e_JobPriority a_priority) : ThreadJob(a_priority)
    {
        m_func = a_func;
    }
    virtual ~FThreadJob() { }

    inline std::future<R> GetFuture()
    {
        return m_promise.get_future();
    }

    virtual void Execute()
    {
        const R val = m_func();

        m_promise.set_value(val);
    }
};