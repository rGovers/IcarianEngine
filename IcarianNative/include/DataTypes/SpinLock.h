// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <atomic>
#include <cstdint>

// May need to change for other platforms but will work for now
// Note that do not actually support MSVC just there for completion sake
#if defined(MSVC) && defined(__x86_64__)
#define ISPINPAUSE _mm_pause()
#elif defined(__GNUC__) && defined (__x86_64__)
#define ISPINPAUSE __builtin_ia32_pause()
#else
// Fallback do not want because of syscalls
#include <thread>
#define ISPINPAUSE std::this_thread::yield()
#endif

// Mutexs are bloated and do not need all the funcionality of them
// Credit as figuring out flags is a pain for optimization: https://rigtorp.se/spinlock/
class SpinLock
{
private:
    volatile std::atomic<bool> m_state;
    
protected:

public:
    SpinLock()
    {
        m_state = false;
    }
    ~SpinLock()
    {

    }

    void Lock()
    {
        while (true)
        {
            if (!m_state.exchange(true, std::memory_order_acquire))
            {
                break;
            }

            while (m_state.load(std::memory_order_relaxed)) 
            {
                ISPINPAUSE;
            }
        }
    }
    inline void Unlock()
    {
        m_state.store(false, std::memory_order_release);
    }
};

class SharedSpinLock
{
private:
    volatile std::atomic<uint32_t> m_read;
    volatile std::atomic<bool>     m_write;

protected:

public:
    SharedSpinLock()
    {
        m_read = 0;
        m_write = false;
    }
    ~SharedSpinLock()
    {

    }

    void Lock()
    {
        while (true)
        {
            if (!m_write.exchange(true, std::memory_order_acquire))
            {
                break;
            }

            while (m_write.load(std::memory_order_relaxed)) 
            {
                ISPINPAUSE;
            }
        }

        while (m_read.load(std::memory_order_seq_cst) > 0) 
        { 
            ISPINPAUSE; 
        }
    }
    inline void Unlock()
    {
        m_write.store(false, std::memory_order_release);
    }

    void LockShared()
    {
        while (true)
        {
            while (m_write.load(std::memory_order_seq_cst)) 
            {
                ISPINPAUSE;
            }

            m_read.fetch_add(1, std::memory_order_acquire);

            // Aquired while writing value
            if (m_write.load(std::memory_order_seq_cst))
            {
                m_read.fetch_sub(1, std::memory_order_release);
            }
            else
            {
                return;
            }
        }
    }
    inline void UnlockShared()
    {
        m_read.fetch_sub(1, std::memory_order_release);
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