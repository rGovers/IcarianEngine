#pragma once

#include <cstdint>

class Random
{
private:
    constexpr static uint32_t BufferSize = 2048;

    uint8_t  m_buffer[BufferSize];
    uint32_t m_index;

    void FillBuffer();
    
    Random();

protected:

public:
    ~Random();

    static void Init();
    static void Destroy();

    static uint8_t* GetBytes(uint32_t a_size);

    static uint32_t Range(uint32_t a_min, uint32_t a_max);
    static float Range(float a_min, float a_max);
    static int32_t Range(int32_t a_min, int32_t a_max);
};