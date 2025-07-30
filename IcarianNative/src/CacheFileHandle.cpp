// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "FileHandles/CacheFileHandle.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <cstring>

#include "Core/IcarianDefer.h"

CacheFileHandle::CacheFileHandle(FileBuffer* a_buffer)
{
    m_offset = 0;

    m_buffer = a_buffer;
    m_buffer->Lock.fetch_add(1);
}
CacheFileHandle::~CacheFileHandle()
{
    if (m_buffer->Lock.fetch_sub(1) <= 1)
    {
        m_buffer->TimePoint = std::chrono::high_resolution_clock::now();
    }
}

uint64_t CacheFileHandle::GetSize() const
{
    return m_buffer->Size;
}
uint64_t CacheFileHandle::GetOffset() const
{
    return m_offset;
}
uint64_t CacheFileHandle::Read(void* a_data, uint64_t a_size)
{
    const uint64_t remaining = m_buffer->Size - m_offset;
    const uint64_t read = glm::min(a_size, remaining);
    IDEFER(m_offset += read);

    memcpy(a_data, (uint8_t*)m_buffer->Data + m_offset, read);

    return read;
}
bool CacheFileHandle::Seek(uint64_t a_offset)
{
    m_offset = glm::min(a_offset, m_buffer->Size);

    return true;
}
bool CacheFileHandle::Ignore(uint64_t a_size)
{
    const uint64_t remaining = m_buffer->Size - m_offset;
    m_offset += glm::min(remaining, a_size);

    return true;
}
bool CacheFileHandle::EndOfFile() const
{
    const uint64_t remaining = m_buffer->Size - m_offset;

    return remaining == 0;
}

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
