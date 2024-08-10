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