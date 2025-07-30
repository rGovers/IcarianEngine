// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "FileHandles/PipeFileHandle.h"

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <cstring>

#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "FileCache.h"
#include "IcarianError.h"

PipeFileHandle::PipeFileHandle(const char* a_path)
{
    memset(m_path, 0, sizeof(m_path));

    for (uint32_t i = 0; i < sizeof(m_path) - 1; ++i)
    {
        if (a_path[i] == 0)
        {
            break;
        }

        m_path[i] = a_path[i];
    }

    m_size = 0;
    m_offset = 0;

    FileCache::SubmitPipeRequest(IcarianCore::PipefileDataType_Size, m_path);

    uint32_t bufferSize;
    uint8_t* data;
    // Get the size ahead of time as it is used by both the user an internally so worthwhile getting
    const e_PipeFileError error = FileCache::AwaitPipeData(IcarianCore::PipefileDataType_Size, m_path, &bufferSize, &data);
    IVERIFY(data != nullptr);
    IDEFER(FileCache::FreePipeData());

    if (error != PipeFileError_Success)
    {
        // This smells I should probably add an error state for the file
        m_size = 0;
    }
    else
    {
        if (bufferSize == sizeof(uint32_t) && data != nullptr)
        {
            m_size = *((uint32_t*)data);
        }
    }
}
PipeFileHandle::~PipeFileHandle()
{

}

uint64_t PipeFileHandle::GetSize() const
{
    return m_size;
}
uint64_t PipeFileHandle::GetOffset() const
{
    return m_offset;
}
uint64_t PipeFileHandle::Read(void* a_data, uint64_t a_size)
{
    IERRBLOCK;

    // TODO: I should probably just kill 64 bit support or make everything 64 bit as it creates too many gotchas
    IVERIFY(a_size < std::numeric_limits<uint32_t>::max());

    // Should not occur but do not waste time if we get a zero size
    if (a_size == 0)
    {
        return 0;
    }

    FileCache::SubmitPipeRequest(IcarianCore::PipefileDataType_Read, m_path, (uint32_t)a_size, m_offset);

    uint32_t size = 0;
    while (true)
    {
        uint32_t readBytes;
        uint8_t* readBuffer;
        const e_PipeFileError error = FileCache::AwaitPipeData(IcarianCore::PipefileDataType_Read, m_path, m_offset, &readBytes, &readBuffer);
        IVERIFY(readBuffer != nullptr);
        IDEFER(FileCache::FreePipeData());

        // Want to break early if it is an error
        IERRCHECKRET(error != PipeFileError_Invalid, 0);

        memcpy((uint8_t*)a_data + size, readBuffer, readBytes);
        size += readBytes;

        // If it is not a partial read we are at the end of the read so break
        if (error == PipeFileError_Success)
        {
            break;
        }
    }

    m_offset = glm::min(m_size, m_offset + size);

    return size;
}
bool PipeFileHandle::Seek(uint64_t a_offset)
{
    IVERIFY(a_offset < std::numeric_limits<uint32_t>::max());

    m_offset = glm::min(m_size, (uint32_t)a_offset);

    return true;
}
bool PipeFileHandle::Ignore(uint64_t a_size)
{
    IVERIFY(a_size < std::numeric_limits<uint32_t>::max());

    const uint32_t remaining = m_size - m_offset;
    m_offset += glm::min(remaining, (uint32_t)a_size);

    return true;
}
bool PipeFileHandle::EndOfFile() const
{
    return m_size == m_offset;
}

#endif

// MIT License
//
// Copyright (c) 2025 River Govers
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
