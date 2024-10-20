// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "FileCache.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <cstring>

#include "Core/IcarianDefer.h"
#include "DataTypes/ThreadGuard.h"
#include "IcarianError.h"
#include "Trace.h"

static FileCache* Instance = nullptr;

// KiB = 1024 bytes 
// MiB = 1024 KiB
// Therefore KiB = B * 1024
// MiB = KiB * 1024
// 1024 = 10th bit
// Therefore 2 1024 Multiplications 20th bit
// Therefore 20
// Note that the i is important otherwise
// KB = 1000 bytes
// MB = 1000 KB
// Throws off the maths
// Table for reference hopefully bit fiddling makes sense now and people stop asking questions
// Use a binary calculator if you are not sure just algebra but binary
// I am probably gonna end up multiplying cause "Code Style" anyway ahhhhhh....
// ------------------------------------------------------------------------------------------------------------------------------------
//                                                  KiB                                                                            MiB
// 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256 | 512 | 1024 | 2048 | 4096 | 8192 | 16384 | 32768 | 65536 | 131072 | 262144 | 524288 | 1048576
// 0 | 1 | 2 | 3 | 4  | 5  | 6  | 7   | 8   | 9   | 10   | 11   | 12   | 13   | 14    | 15    | 16    | 17     | 18     | 19     | 20
constexpr uint32_t MiBToByteShift = 20;

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

ReadFileHandle::ReadFileHandle(FILE* a_file, uint64_t a_size)
{
    m_file = a_file;
    m_size = a_size;
}
ReadFileHandle::~ReadFileHandle()
{
    fclose(m_file);
}

uint64_t ReadFileHandle::GetSize() const
{
    return m_size;
}
uint64_t ReadFileHandle::GetOffset() const
{
    return (uint64_t)ftell(m_file);
}
uint64_t ReadFileHandle::Read(void* a_data, uint64_t a_size)
{
    return (uint64_t)fread(a_data, 1, (size_t)a_size, m_file);
}
bool ReadFileHandle::Seek(uint64_t a_offset)
{
    return fseek(m_file, (long)a_offset, SEEK_SET) == 0;
}
bool ReadFileHandle::Ignore(uint64_t a_size)
{
    return fseek(m_file, (long)a_size, SEEK_CUR) == 0;
}
bool ReadFileHandle::EndOfFile() const
{
    return feof(m_file) != 0;
}

FileCache::FileCache(uint32_t a_sizeMiB)
{
    m_size = (uint64_t)a_sizeMiB << MiBToByteShift;
    m_allocated = 0;
}
FileCache::~FileCache()
{
    for (const auto iter : m_files)
    {
        FileBuffer* buffer = iter.second;
        delete[] (uint8_t*)buffer->Data;
        delete buffer;
    }
}

void FileCache::Init(uint32_t a_sizeMB)
{
    if (Instance == nullptr)
    {
        Instance = new FileCache(a_sizeMB);
    }
}
void FileCache::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

static FileBuffer* GenerateFileBuffer(FILE* a_file, uint64_t a_size)
{
    FileBuffer* buffer = new FileBuffer();
    buffer->Size = a_size;
    buffer->Data = new uint8_t[a_size];
    fread(buffer->Data, (size_t)a_size, 1, a_file);
    buffer->TimePoint = std::chrono::high_resolution_clock::now();
    buffer->Lock = 0;

    return buffer;
}

FileHandle* FileCache::GenerateFileHandle(const std::filesystem::path& a_path, FILE* a_file, uint64_t a_size)
{
    const uint64_t remaining = m_size - m_allocated;

    if (a_size < remaining)
    {
        IDEFER(fclose(a_file));

        FileBuffer* buffer = GenerateFileBuffer(a_file, a_size);        

        m_allocated += a_size;

        m_files.emplace(a_path, buffer);

        return new CacheFileHandle(buffer);
    }

    std::filesystem::path key;
    FileBuffer* b = nullptr;
    // Looking for a file to delete that so that the current file can fit
    // No point deleting a file if we cannot fit it
    for (const auto iter : m_files)
    {
        FileBuffer* buffer = iter.second;
        if (buffer->Size >= a_size && buffer->Lock == 0)
        {
            if (b == nullptr)
            {
                key = iter.first;
                b = buffer;

                continue;
            }

            if (buffer->TimePoint > b->TimePoint)
            {
                continue;
            }

            key = iter.first;
            b = buffer;
        }
    }

    if (b == nullptr)
    {
        return new ReadFileHandle(a_file, a_size);
    }

    IDEFER(fclose(a_file));

    m_files.erase(key);
    m_allocated -= b->Size;
    delete[] (uint8_t*)b->Data;
    delete b;

    b = GenerateFileBuffer(a_file, a_size);

    m_files.emplace(a_path, b);
    m_allocated += b->Size;

    return new CacheFileHandle(b);
}

void FileCache::Update()
{
    if (Instance == nullptr)
    {
        return;
    }

    Instance->m_updateFrame = (Instance->m_updateFrame + 1) % 4;
    // Want to clear it if we are over half full but do not need to do it regularly
    if (Instance->m_updateFrame != 0)
    {
        return;
        
    }

    const uint64_t halfSize = Instance->m_size >> 1;
    if (Instance->m_allocated < halfSize)
    {
        return;
    }

    const std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

    const ThreadGuard g = ThreadGuard(Instance->m_lock);

    const uint32_t mapSize = (uint32_t)Instance->m_files.size();
    if (mapSize == 0)
    {
        return;
    }

    std::filesystem::path* keys = new std::filesystem::path[mapSize];
    IDEFER(delete[] keys);
    uint32_t keyCount = 0;

    for (const auto iter : Instance->m_files) 
    {
        const FileBuffer* buffer = iter.second;
        if (buffer->Lock != 0) 
        {
            continue;
        }

        const float timePassed = std::chrono::duration<float>(now - buffer->TimePoint).count();
        if (timePassed > 1.0f) 
        {
            keys[keyCount++] = iter.first;
        }
    }

    if (keyCount <= 0)
    {
        return;
    }

    TRACE("Flushing File Cache");

    for (uint32_t i = 0; i < keyCount; ++i)
    {
        const std::filesystem::path& key = keys[i];

        const FileBuffer* buffer = Instance->m_files[key];
        IDEFER(
        {
            delete[] (uint8_t*)buffer->Data;
            delete buffer;
        });

        Instance->m_files.erase(key);
    }
}

void FileCache::PreLoad(const std::filesystem::path& a_path)
{
    if (Instance == nullptr)
    {
        return;
    }

    {
        const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

        const auto iter = Instance->m_files.find(a_path);
        if (iter != Instance->m_files.end())
        {
            return;
        }
    }

    const ThreadGuard g = ThreadGuard(Instance->m_lock);

    const std::string s = a_path.string();
    const uint64_t maxSize = Instance->m_size >> 3;

    FILE* fp = fopen(s.c_str(), "rb");
    if (fp != NULL)
    {
        fseek(fp, 0L, SEEK_END);
        const uint64_t size = (uint64_t)ftell(fp);
        rewind(fp);

        if (size >= maxSize)
        {
            IDEFER(fclose(fp));

            return;
        }

        // Not the most efficent but I am lazy
        FileHandle* handle = Instance->GenerateFileHandle(a_path, fp, size);
        IDEFER(delete handle);
    }
    else
    {
        IERROR("Unable to open file: " + s);
    }
}
FileHandle* FileCache::LoadFile(const std::filesystem::path& a_path)
{
    if (Instance != nullptr)
    {
        {
            const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

            const auto iter = Instance->m_files.find(a_path);
            if (iter != Instance->m_files.end())
            {
                return new CacheFileHandle(iter->second);
            }
        }
        
        const ThreadGuard g = ThreadGuard(Instance->m_lock);

        const std::string s = a_path.string();
        // Do not want the whole cache taken up by a single file otherwise it eliminates the point
        const uint64_t maxSize = Instance->m_size >> 3;

        // I am aware the there is a C++ version for file handles, however been having issues on 
        // Windows so minimizing the layers of abstraction to reduce points of failure.
        // Yes I am aware of what I am saying, I am implementing a layer of abstraction hush.
        // Have only had issues on Windows, Linux has been fine.
        FILE* fp = fopen(s.c_str(), "rb");
        if (fp != NULL)
        {
            fseek(fp, 0L, SEEK_END);
            const uint64_t size = (uint64_t)ftell(fp);
            rewind(fp);

            // If the file is massive dont bother storing it in the cache just pass it through
            if (size >= maxSize)
            {
                return new ReadFileHandle(fp, size);
            }

            return Instance->GenerateFileHandle(a_path, fp, size);
        }
        else
        {
            IERROR("Unable to open file: " + s);
        }
    }
    // May not want cache on all platforms but still need file IO and prefer minimize duplicate code
    else
    {
        const std::string s = a_path.string();

        FILE* fp = fopen(s.c_str(), "rb");
        if (fp != NULL)
        {
            fseek(fp, 0L, SEEK_END);
            const uint64_t size = (uint64_t)ftell(fp);
            rewind(fp);

            return new ReadFileHandle(fp, size);
        }
        else 
        {
            IERROR("Unable to open file: " + s);
        }
    }

    return nullptr;
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