#pragma once

#include <cstdint>
#include <cstdlib>

class ChunkAllocator
{
private:
    void* m_memory;
    void* m_slider;
    void* m_end;

protected:

public:
    ChunkAllocator(uint64_t a_size)
    {
        // Can probably use os specific allocation here eg. mmap on linux or VirtualAlloc on windows but lazy
        m_memory = malloc(a_size);
        m_slider = m_memory;
        m_end = (void*)((char*)m_memory + a_size);
    }
    ~ChunkAllocator()
    {
        free(m_memory);
    }

    inline uint64_t GetSize() const
    {
        return (uint64_t)((char*)m_end - (char*)m_memory);
    }
    inline uint64_t GetUsed() const
    {
        return (uint64_t)((char*)m_slider - (char*)m_memory);
    }
    inline uint64_t GetFree() const
    {
        return (uint64_t)((char*)m_end - (char*)m_slider);
    }

    void* Allocate(uint64_t a_size)
    {
        if ((char*)m_slider + a_size > (char*)m_end)
        {
            return nullptr;
        }

        void* result = m_slider;
        m_slider = (void*)((char*)m_slider + a_size);
        return result;
    }

    template<typename T>
    inline T* Allocate()
    {
        return (T*)Allocate(sizeof(T));
    }
    template<typename T>
    inline T* Allocate(uint64_t a_count)
    {
        return (T*)Allocate(sizeof(T) * a_count);
    }

    inline void Reset()
    {
        m_slider = m_memory;
    }
};