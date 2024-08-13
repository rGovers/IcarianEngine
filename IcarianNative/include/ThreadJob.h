// Icarian Engine - C# Game Engine
// 
// License at end of file.

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