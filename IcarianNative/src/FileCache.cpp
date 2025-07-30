// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "FileCache.h"

#include <cstring>
#include <thread>

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
#ifndef WIN32
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#endif

#include "Core/Bitfield.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"
#include "Core/StringUtils.h"
#include "DataTypes/ThreadGuard.h"
#include "FileHandles/ReadFileHandle.h"
#include "IcarianError.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
#include "FileHandles/PipeFileHandle.h"
#endif

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

RUNTIME_FUNCTION(uint32_t, FileCache, ExistingFile,
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    return FileCache::Exists(str);
}, MonoString* a_path)
RUNTIME_FUNCTION(uint32_t, FileCache, CachedFile,
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    return FileCache::ExistsInCache(str);
}, MonoString* a_path)
RUNTIME_FUNCTION(MonoArray*, FileCache, ReadFileData, 
{
    IERRBLOCK;
    
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    FileHandle* handle = FileCache::LoadFile(str);
    IERRCHECKRET(handle != nullptr, NULL);
    IDEFER(delete handle);

    const uint64_t size = handle->GetSize();

    uint8_t* dat = new uint8_t[size];
    IDEFER(delete[] dat);

    IERRCHECKRET(handle->Read(dat, size) == size, NULL);

    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_byte_class(), (uintptr_t)size);
    for (uint64_t i = 0; i < size; ++i)
    {
        mono_array_set(arr, mono_byte, i, dat[i]);
    }

    return arr;
}, MonoString* a_path)
RUNTIME_FUNCTION(void, FileCache, WriteFileData, 
{
    char* str = mono_string_to_utf8(a_path);
    IDEFER(mono_free(str));

    const uint64_t size = mono_array_length(a_data);
    uint8_t* dat = new uint8_t[size];

    for (uint64_t i = 0; i < size; ++i)
    {
        dat[i] = mono_array_get(a_data, uint8_t, i);
    }

    if (a_writeFile)
    {
        FILE* file = fopen(str, "wb");
        if (file != NULL)
        {
            IDEFER(fclose(file));

            fwrite(dat, 1, size, file);
        }
    }

    FileCache::PushFile(str, dat, (uint32_t)size, (bool)a_pinFile);
}, MonoString* a_path, MonoArray* a_data, uint32_t a_writeFile, uint32_t a_pinFile)

FileCache::FileCache(uint32_t a_sizeMiB, uint32_t a_pipefileID)
{
    IERRBLOCK;

    m_size = (uint64_t)a_sizeMiB << MiBToByteShift;
    m_allocated = 0;

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    TRACE("Initializing pipefile");
    m_pipefileID = a_pipefileID;

    m_readBuffer = malloc(SharedBufferSize);

    m_commandBuffer = NULL;
    m_dataBuffer = NULL;

    const std::string idStr = std::to_string(m_pipefileID);

    const std::string cmdStr = CommandBufferName + idStr;
    const std::string dataStr = DataBufferName + idStr;

    // TODO: Implement Windows version of this
#ifndef WIN32
    const int commandFd = shm_open(cmdStr.c_str(), O_RDWR, 0);
    if (commandFd < 0)
    {
        Logger::Error("Failed to initialize Pipefile falling back to FileIO");

        ITRIGGERERR;
    }
    IDEFER(close(commandFd));
    IERRDEFER(shm_unlink(cmdStr.c_str()));

    m_commandBuffer = (IcarianCore::SharedMemoryBuffer*)mmap(NULL, (size_t)SharedBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, commandFd, 0);
    if (m_commandBuffer == MAP_FAILED || m_commandBuffer == NULL)
    {
        IERROR("Failed to map pipefile command shared memory buffer");
    }
    IERRDEFER(
    {
        munmap(m_commandBuffer, SharedBufferSize);
        m_commandBuffer = NULL;
    });

    const int dataFd = shm_open(dataStr.c_str(), O_RDWR, 0);
    if (dataFd < 0)
    {
        Logger::Error("Failed to initialize Pipefile falling back to FileIO");

        ITRIGGERERR;
    }
    IDEFER(close(dataFd));
    IERRDEFER(shm_unlink(dataStr.c_str()));

    m_dataBuffer = (IcarianCore::SharedMemoryBuffer*)mmap(NULL, (size_t)SharedBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, dataFd, 0);
    if (m_dataBuffer == MAP_FAILED || m_dataBuffer == NULL)
    {
        IERROR("Failed to map pipefile data shared memory buffer");
    }
    IERRDEFER(
    {
        munmap(m_dataBuffer, SharedBufferSize);
        m_dataBuffer = NULL;
    });
#endif
#endif
}
FileCache::~FileCache()
{
    for (const auto& iter : m_files)
    {
        const FileBuffer* buffer = iter.second;

        delete[] (uint8_t*)buffer->Data;
        delete buffer;
    }

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    free(m_readBuffer);

    const std::string idStr = std::to_string(m_pipefileID);

    const std::string cmdStr = CommandBufferName + idStr;
    const std::string dataStr = DataBufferName + idStr;

#ifndef WIN32
    if (m_commandBuffer != NULL)
    {
        munmap(m_commandBuffer, SharedBufferSize);
        m_commandBuffer = NULL;

        shm_unlink(cmdStr.c_str());
    }

    if (m_dataBuffer != NULL)
    {
        munmap(m_dataBuffer, SharedBufferSize);
        m_dataBuffer = NULL;

        shm_unlink(dataStr.c_str());
    }
#endif
#endif
}

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
static IcarianCore::PipefileHeader* GetPipefileHeader(const IcarianCore::SharedMemoryBuffer* a_buffer)
{
    return (IcarianCore::PipefileHeader*)((uint8_t*)a_buffer + sizeof(IcarianCore::SharedMemoryBuffer));
}
static uint8_t* GetDataSection(const IcarianCore::PipefileHeader* a_header)
{
    return (uint8_t*)a_header + sizeof(IcarianCore::PipefileHeader);
}
#endif

void FileCache::Init(uint32_t a_sizeMB, uint32_t a_pipefileID)
{
    if (Instance == nullptr)
    {
        Instance = new FileCache(a_sizeMB, a_pipefileID);
    }

    // Not having a FileCache is valid so init the functions out here
    BIND_FUNCTION(IcarianEngine, FileCache, ExistingFile);
    BIND_FUNCTION(IcarianEngine, FileCache, CachedFile);
    BIND_FUNCTION(IcarianEngine, FileCache, ReadFileData);
    BIND_FUNCTION(IcarianEngine, FileCache, WriteFileData);
}
void FileCache::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

static FileBuffer* GenerateFileBuffer(FileHandle* a_file)
{
    const uint64_t size = a_file->GetSize();

    FileBuffer* buffer = new FileBuffer();
    buffer->Size = size;
    buffer->Data = new uint8_t[size];
    if (a_file->Read(buffer->Data, size) != size)
    {
        IERROR("Failed to read FileHandle into FileBuffer");
    }
    buffer->TimePoint = std::chrono::high_resolution_clock::now();
    buffer->Lock = 0;
    buffer->Flags = 0;

    return buffer;
}

void FileCache::SubmitPipeRequest(IcarianCore::e_PipefileDataType a_type, const std::string_view& a_path, uint32_t a_size, uint32_t a_offset)
{
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    if (Instance == nullptr)
    {
        IERROR("Making pipe request with no FileCache");
    }

    if (Instance->m_commandBuffer == NULL || Instance->m_dataBuffer == NULL)
    {
        IERROR("Making pipe request with no pipe");
    }

    // TODO: I should probably build a work queue so I can be smarter about awaiting
    const size_t pathSize = a_path.size();
    if (pathSize <= 0)
    {
        IERROR("Pipe request with empty string");
    }

    if (pathSize >= (IcarianCore::PipefileHeader::MaxPathSize - 1))
    {
        IERROR("Pipe path size greater then max path size");
    }

    const std::unique_lock g = std::unique_lock(Instance->m_commandPipelock);

    while (Instance->m_commandBuffer->State != IcarianCore::SharedMemoryBufferState_Write)
    {
        // The other side of the buffer is probably busy so just wait for it to be populated
        // This is mostly so the other process can get access to resources incase it is us hogging them
        std::this_thread::yield();
    }

    IVERIFY(Instance->m_commandBuffer->Size >= sizeof(IcarianCore::PipefileHeader));
    IDEFER(Instance->m_commandBuffer->State = IcarianCore::SharedMemoryBufferState_Read);

    IcarianCore::PipefileHeader* header = GetPipefileHeader(Instance->m_commandBuffer);
    header->Type = a_type;
    header->Size = a_size;
    header->Offset = a_offset;
    header->Partial = false;
    a_path.copy(header->Path, pathSize, 0);
    header->Path[pathSize] = 0;
#endif
}
e_PipeFileError FileCache::AwaitPipeData(IcarianCore::e_PipefileDataType a_type, const std::string_view& a_path, uint32_t* a_bufferSize, uint8_t** a_data)
{
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    return AwaitPipeData(a_type, a_path, 0, a_bufferSize, a_data);
#else
    return PipeFileError_Invalid;
#endif
}
e_PipeFileError FileCache::AwaitPipeData(IcarianCore::e_PipefileDataType a_type, const std::string_view& a_path, uint32_t a_offset, uint32_t* a_bufferSize, uint8_t** a_data)
{
    IVERIFY(a_bufferSize != nullptr);
    IVERIFY(a_data != nullptr);

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    if (Instance == nullptr)
    {
        IERROR("Awaiting pipe data with no FileCache");
    }

    if (Instance->m_commandBuffer == NULL || Instance->m_dataBuffer == NULL)
    {
        IERROR("Awaiting pipe data with no pipe");
    }

    *a_data = nullptr;

    // TODO: This is fucking terrible need a way for it to fail as there is edge cases that this can stall out and be stuck in an infinite loop
    // Good news I am not short on rugs or brooms atleast so not high priority
    while (true)
    {
        {
            const std::unique_lock g = std::unique_lock(Instance->m_dataPipelock);

            if (Instance->m_dataBuffer->State == IcarianCore::SharedMemoryBufferState_Read)
            {
                IVERIFY(Instance->m_dataBuffer->Size > sizeof(IcarianCore::PipefileHeader));

                IcarianCore::PipefileHeader* header = GetPipefileHeader(Instance->m_dataBuffer);

                if (header->Type == IcarianCore::PipefileDataType_Invalid)
                {
                    IVERIFY(header->Size == sizeof(IcarianCore::PipefileHeader));

                    volatile IcarianCore::PipefileHeader* invalidHeader = (IcarianCore::PipefileHeader*)GetDataSection(header);

                    if (invalidHeader->Type == a_type && invalidHeader->Offset == a_offset && header->Path == a_path)
                    {
                        return PipeFileError_Invalid;
                    }
                }

                if (header->Type == a_type && header->Offset == a_offset && header->Path == a_path)
                {
                    Instance->m_readLock.lock();

                    IDEFER(Instance->m_dataBuffer->State = IcarianCore::SharedMemoryBufferState_Write);

                    const uint32_t size = header->Size;
                    if (size > SharedBufferSize - sizeof(IcarianCore::PipefileHeader) - sizeof(IcarianCore::SharedMemoryBuffer))
                    {
                        IERROR("Pipe await overflow");
                    }

                    volatile uint8_t* data = GetDataSection(header);

                    // TODO: Look into having the FileCache hold a buffer and handing it back to the FileCache when done over allocating and deallocating
                    // We know the max size ahead of time so probably an effective stratergy
                    *a_data = (uint8_t*)Instance->m_readBuffer;

                    for (uint32_t i = 0; i < size; ++i)
                    {
                        (*a_data)[i] = data[i];
                    }
                    *a_bufferSize = size;

                    if (header->Partial)
                    {
                        // Want to inform the user it is a partial header to get them to recall this function
                        return PipeFileError_Partial;
                    }

                    return PipeFileError_Success;
                }
            }
        }

        // Either we are busy on another thread or the other process is busy
        // Both instances release the thread to reduce contention
        std::this_thread::yield();
    }
#endif

    return PipeFileError_Invalid;
}
void FileCache::FreePipeData()
{
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    Instance->m_readLock.unlock();
#endif
}

bool FileCache::Exists(const std::string_view& a_str)
{
    if (Instance != nullptr)
    {
        const size_t protocolIndex = a_str.find("://");
        if (protocolIndex != std::string_view::npos)
        {
            constexpr uint32_t BufferSize = 16;
            if (protocolIndex >= (BufferSize - 1))
            {
                IERROR("Protocol buffer would overflow");
            }

            char buffer[BufferSize];

            a_str.copy(buffer, protocolIndex, 0);
            buffer[protocolIndex] = 0;

            switch (StringHash<uint32_t>(buffer))
            {
            case StringHash<uint32_t>("pipe"):
            {
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
                if (Instance->m_dataBuffer == NULL || Instance->m_commandBuffer == NULL)
                {
                    return false;
                }

                const std::string_view str = a_str.substr(protocolIndex + 3);

                SubmitPipeRequest(IcarianCore::PipefileDataType_Exists, str);

                uint8_t* data;
                uint32_t dataSize;
                const e_PipeFileError error = AwaitPipeData(IcarianCore::PipefileDataType_Exists, str, &dataSize, &data);

                IVERIFY(data != nullptr);
                IDEFER(FreePipeData());

                if (error != PipeFileError_Success)
                {
                    return false;
                }

                IVERIFY(dataSize == sizeof(uint8_t));

                return *data != 0;
#else
                return false;
#endif

                break;
            }
            default:
            {
                IERROR("Invalid protocol");

                break;
            }
            }
        }
    }

    return std::filesystem::exists(a_str);
}
bool FileCache::ExistsInCache(const std::string_view& a_str)
{
    const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

    return Instance->m_files.find(std::string(a_str)) != Instance->m_files.end();
}

void FileCache::PushFile(const std::string_view& a_path, uint8_t* a_data, uint32_t a_size, bool a_pin)
{
    const std::string str = std::string(a_path);

    const ThreadGuard g = ThreadGuard(Instance->m_lock);

    const auto iter = Instance->m_files.find(str);
    if (iter != Instance->m_files.end())
    {
        FileBuffer* buffer = iter->second;
        while (buffer->Lock > 0)
        {
            std::this_thread::yield();
        }

        Instance->m_allocated -= buffer->Size;
        Instance->m_allocated += a_size;

        delete[] (uint8_t*)buffer->Data;

        buffer->Size = a_size;
        buffer->Data = a_data;
        buffer->TimePoint = std::chrono::high_resolution_clock::now();
        buffer->Flags = 0;

        if (a_pin)
        {
            ISETBIT(buffer->Flags, FileBuffer::PinnedBit);
        }

        return;
    }

    Instance->m_allocated += a_size;

    FileBuffer* buffer = new FileBuffer();
    buffer->Size = a_size;
    buffer->Data = a_data;
    buffer->TimePoint = std::chrono::high_resolution_clock::now();
    buffer->Lock = 0;
    buffer->Flags = 0;

    if (a_pin)
    {
        ISETBIT(buffer->Flags, FileBuffer::PinnedBit);
    }

    Instance->m_files.emplace(str, buffer);
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

    std::string* keys = new std::string[mapSize];
    IDEFER(delete[] keys);
    uint32_t keyCount = 0;

    for (const auto& iter : Instance->m_files) 
    {
        const FileBuffer* buffer = iter.second;
        if (IISBITSET(buffer->Flags, FileBuffer::PinnedBit))
        {
            continue;
        }

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
        const std::string& key = keys[i];

        // Use at otherwise does not compile in the Steam Sniper runtime
        const FileBuffer* buffer = Instance->m_files.at(key);
        IDEFER(
        {
            delete[] (uint8_t*)buffer->Data;
            delete buffer;
        });

        Instance->m_files.erase(key);
    }
}

FileHandle* FileCache::LoadFile(const std::string_view& a_path)
{
    if (Instance == nullptr)
    {
        return ReadFileHandle::OpenFile(a_path);
    }

    const std::string pathStr = std::string(a_path);

    {
        const SharedThreadGuard g = SharedThreadGuard(Instance->m_lock);

        const auto iter = Instance->m_files.find(pathStr);
        if (iter != Instance->m_files.end())
        {
            return new CacheFileHandle(iter->second);
        }
    }

    const ThreadGuard g = ThreadGuard(Instance->m_lock);

    FileHandle* handle = ILAMBDA(
    {
        const size_t protocolIndex = a_path.find("://");
        if (protocolIndex != std::string_view::npos)
        {
            constexpr uint32_t BufferSize = 16;
            if (protocolIndex >= (BufferSize - 1))
            {
                IERROR("Protocol buffer would overflow");
            }

            char buffer[BufferSize];

            a_path.copy(buffer, protocolIndex, 0);
            buffer[protocolIndex] = 0;

            switch (StringHash<uint32_t>(buffer))
            {
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
            case StringHash<uint32_t>("pipe"):
            {
                const std::string_view str = a_path.substr(protocolIndex + 3);

                ILRETURN (FileHandle*)new PipeFileHandle(str.data());
            }
#endif
            default:
            {
                ILRETURN (FileHandle*)nullptr;
            }
            }
        }

        ILRETURN (FileHandle*)ReadFileHandle::OpenFile(a_path);
    });

    if (handle == nullptr)
    {
        return nullptr;
    }

    const uint64_t maxSize = Instance->m_size >> 3;
    const uint64_t size = handle->GetSize();
    if (size >= maxSize)
    {
        return handle;
    }

    if (Instance->m_allocated < Instance->m_size && size < Instance->m_size - Instance->m_allocated)
    {
        IDEFER(delete handle);

        FileBuffer* buffer = GenerateFileBuffer(handle);

        Instance->m_allocated += size;

        Instance->m_files.emplace(a_path, buffer);

        return new CacheFileHandle(buffer);
    }

    const uint64_t offsetSize = ILAMBDA(
    {
        if (Instance->m_allocated > Instance->m_size)
        {
            ILRETURN Instance-> m_allocated - Instance->m_size;
        }

        ILRETURN uint64_t(0);
    });

    const uint64_t finalSize = offsetSize + size;

    std::string key;
    FileBuffer* b = nullptr;
    // Looking for a file to delete that so that the current file can fit
    // No point deleting a file if we cannot fit it
    for (const auto& iter : Instance->m_files)
    {
        FileBuffer* buffer = iter.second;
        if (IISBITSET(buffer->Flags, FileBuffer::PinnedBit))
        {
            continue;
        }

        if (buffer->Size >= finalSize && buffer->Lock == 0)
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
        return handle;
    }

    IDEFER(delete handle);

    Instance->m_files.erase(key);
    Instance->m_allocated -= b->Size;
    delete[] (uint8_t*)b->Data;
    delete b;

    b = GenerateFileBuffer(handle);

    Instance->m_files.emplace(a_path, b);
    Instance->m_allocated += b->Size;

    return new CacheFileHandle(b);
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
