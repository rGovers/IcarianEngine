// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Random.h"

#include "IcarianError.h"
#include "Runtime/RuntimeManager.h"

static Random* Instance = nullptr;

RUNTIME_FUNCTION(void, Random, FillBuffer,
{
    const uint32_t size = mono_array_length(a_buffer);
    const uint32_t fillSize = size - a_offset;

    const uint8_t* bytes = Random::GetBytes(fillSize);

    for (uint32_t i = 0; i < fillSize; ++i)
    {
        mono_array_set(a_buffer, mono_byte, i + a_offset, bytes[i]);
    }
}, MonoArray* a_buffer, uint32_t a_offset)

Random::Random()
{
    m_index = BufferSize;
    
    FillBuffer();
}
Random::~Random()
{
    
}

void Random::Init()
{
    if (Instance == nullptr)
    {
        Instance = new Random();

        BIND_FUNCTION(IcarianEngine, Random, FillBuffer);
    }
}
void Random::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void Random::FillBuffer()
{
    const uint32_t offset = BufferSize - m_index;

    for (uint32_t i = 0; i < offset; ++i)
    {
        m_buffer[i] = m_buffer[m_index + i];
    }

    const uint32_t size = BufferSize - offset;

    // TODO: Use better RNG as rand seems to be bias 
    // rand does not seem to be very good so have to do it at the byte level to increase the noise
    uint8_t* writeBuffer = (uint8_t*)(m_buffer + offset);
    for (uint32_t i = 0; i < size; ++i)
    {
        writeBuffer[i] = (uint8_t)((double)rand() / RAND_MAX * UINT8_MAX);
    }

    m_index = 0;
}

uint8_t* Random::GetBytes(uint32_t a_size)
{
    IVERIFY(a_size < BufferSize);

    if (Instance->m_index + a_size > BufferSize)
    {
        Instance->FillBuffer();
    }

    uint8_t* buffer = Instance->m_buffer + Instance->m_index;
    Instance->m_index += a_size;

    return buffer;
}

uint32_t Random::Range(uint32_t a_min, uint32_t a_max)
{
    if (Instance->m_index + sizeof(uint32_t) >= BufferSize)
    {
        Instance->FillBuffer();
    }

    uint32_t* buffer = (uint32_t*)(Instance->m_buffer + Instance->m_index);
    Instance->m_index += sizeof(uint32_t);

    return a_min + (*buffer % (a_max - a_min));
}
float Random::Range(float a_min, float a_max)
{
    if (Instance->m_index + sizeof(float) >= BufferSize)
    {
        Instance->FillBuffer();
    }

    uint32_t* buffer = (uint32_t*)(Instance->m_buffer + Instance->m_index);
    Instance->m_index += sizeof(uint32_t);

    return (float)(a_min + ((double)*buffer / UINT32_MAX) * (a_max - a_min));
}
int32_t Random::Range(int32_t a_min, int32_t a_max)
{
    if (Instance->m_index + sizeof(int32_t) >= BufferSize)
    {
        Instance->FillBuffer();
    }

    int32_t* buffer = (int32_t*)(Instance->m_buffer + Instance->m_index);
    Instance->m_index += sizeof(int32_t);

    return a_min + (*buffer % (a_max - a_min));
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