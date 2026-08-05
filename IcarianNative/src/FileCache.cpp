// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "FileCache.h"

#include <cstring>
#include <filesystem>
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
#include "Core/DataTypes/Allocators/LeakAllocator.h"
#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/Allocators/MultiSourceAllocator.h"
#include "Core/DataTypes/Allocators/OSAllocator.h"
#include "Core/DataTypes/Allocators/UberAllocator.h"
#include "Core/DataTypes/ThreadGuard.h"
#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"
#include "Core/StringUtils.h"
#include "Core/DataTypes/Allocators/BlockAllocator.h"
#include "FileHandles/ReadFileHandle.h"
#include "IcarianError.h"
#include "Profiler.h"
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
    IDEFER(IcarianCore::MallocAllocator::Instance->Destroy(handle));

    const uint64_t size = handle->GetSize();

    uint8_t* dat = IcarianCore::MallocAllocator::Instance->ZTAllocate<uint8_t>(size);
    IDEFER(IcarianCore::MallocAllocator::Instance->Free(dat));

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
    uint8_t* dat = IcarianCore::MallocAllocator::Instance->ZTAllocate<uint8_t>(size);
    IDEFER(IcarianCore::MallocAllocator::Instance->Destroy(dat));

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

    FileCache::PushFile(IcarianCore::COWU8String(str, IcarianCore::MallocAllocator::Instance), dat, (uint32_t)size, (bool)a_pinFile);
}, MonoString* a_path, MonoArray* a_data, uint32_t a_writeFile, uint32_t a_pinFile)

FileCache::FileCache(uint32_t a_sizeMiB, uint32_t a_pipefileID)
{
    IERRBLOCK;

    m_smallAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::BlockAllocator>(SmallAllocatorSize, IcarianCore::UberAllocator::Instance);
    m_largeAllocator = IcarianCore::MallocAllocator::Instance->Create<IcarianCore::BlockAllocator>(LargeAllocatorSize, IcarianCore::UberAllocator::Instance);

    m_allocatorChain = m_smallAllocator->Create<IcarianCore::Array<IcarianCore::Allocator*>>(m_smallAllocator);

    const IcarianCore::AllocationSource allocatorSources[] =
    {
        {
            .Alloc = m_smallAllocator,
            .MaxSize = SmallAllocatorSize >> 1,
        },
        {
            .Alloc = m_largeAllocator,
            .MaxSize = LargeAllocatorSize >> 1,
        },
        {
            .Alloc = IcarianCore::OSAllocator::Instance,
            .MaxSize = uint64_t(-1),
        },
    };

    constexpr uint32_t AllocatorCount = sizeof(allocatorSources) / sizeof(*allocatorSources);

    m_allocator = m_smallAllocator->Create<IcarianCore::MultiSourceAllocator>(m_smallAllocator, allocatorSources, AllocatorCount);
    m_allocatorChain->Push(m_allocator);

    m_trackerAllocator = m_smallAllocator->Create<IcarianCore::TrackerAllocator>(m_allocator);
    m_allocator = m_trackerAllocator;
    m_allocatorChain->Push(m_allocator);

#ifdef DEBUG
    m_allocator = m_smallAllocator->Create<IcarianCore::LeakAllocator>(m_allocator);
    m_allocatorChain->Push(m_allocator);
#endif

    m_data = m_allocator->ZTAllocate<ClassData>();
    m_data->Files = IcarianCore::Dictionary<IcarianCore::COWU8String, FileBuffer*>(m_allocator);

    m_data->Size = (uint64_t)a_sizeMiB << MiBToByteShift;

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    TRACE("Initializing pipefile");
    m_data->PipefileID = a_pipefileID;

    m_data->ReadBuffer = m_allocator->ZAllocate(SharedBufferSize, 16);

    const IcarianCore::COWU8String idStr = IcarianCore::COWU8String::FromValue(m_data->PipefileID, 10, m_allocator);

    const IcarianCore::COWU8String cmdStr = CommandBufferName + idStr;
    const IcarianCore::COWU8String dataStr = DataBufferName + idStr;

    // TODO: Implement Windows version of this
#ifndef WIN32
    const int commandFd = shm_open(cmdStr.CStr(), O_RDWR, 0);
    if (commandFd < 0)
    {
        Logger::Error("Failed to initialize Pipefile falling back to FileIO");

        ITRIGGERERR;
    }
    IDEFER(close(commandFd));
    IERRDEFER(shm_unlink(cmdStr.CStr()));

    m_data->CommandBuffer = (IcarianCore::SharedMemoryBuffer*)mmap
    (
        NULL,
        (size_t)SharedBufferSize,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        commandFd,
        0
    );
    if (m_data->CommandBuffer == MAP_FAILED || m_data->CommandBuffer == NULL)
    {
        IERROR("Failed to map pipefile command shared memory buffer");
    }
    IERRDEFER(
    {
        munmap(m_data->CommandBuffer, SharedBufferSize);
        m_data->CommandBuffer = NULL;
    });

    const int dataFd = shm_open(dataStr.CStr(), O_RDWR, 0);
    if (dataFd < 0)
    {
        Logger::Error("Failed to initialize Pipefile falling back to FileIO");

        ITRIGGERERR;
    }
    IDEFER(close(dataFd));
    IERRDEFER(shm_unlink(dataStr.CStr()));

    m_data->DataBuffer = (IcarianCore::SharedMemoryBuffer*)mmap
    (
        NULL,
        (size_t)SharedBufferSize,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        dataFd,
        0
    );
    if (m_data->DataBuffer == MAP_FAILED || m_data->DataBuffer == NULL)
    {
        IERROR("Failed to map pipefile data shared memory buffer");
    }
    IERRDEFER(
    {
        munmap(m_data->DataBuffer, SharedBufferSize);
        m_data->DataBuffer = NULL;
    });
#endif
#endif
}
FileCache::~FileCache()
{
    {
        const IcarianCore::Array<FileBuffer*> buffer = m_data->Files.GetValues(m_allocator);

        for (FileBuffer* b : buffer)
        {
            m_allocator->Free(b->Data);
            m_allocator->Destroy(b);
        }
    }

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    {
        m_allocator->Free(m_data->ReadBuffer);

        const IcarianCore::COWU8String idStr = IcarianCore::COWU8String::FromValue(m_data->PipefileID, 10, m_allocator);

        const IcarianCore::COWU8String cmdStr = CommandBufferName + idStr;
        const IcarianCore::COWU8String dataStr = DataBufferName + idStr;

#ifndef WIN32
        if (m_data->CommandBuffer != NULL)
        {
            munmap(m_data->CommandBuffer, SharedBufferSize);
            m_data->CommandBuffer = NULL;

            shm_unlink(cmdStr.CStr());
        }

        if (m_data->DataBuffer != NULL)
        {
            munmap(m_data->DataBuffer, SharedBufferSize);
            m_data->DataBuffer = NULL;

            shm_unlink(dataStr.CStr());
        }
#endif
    }
#endif

    m_allocator->Destroy(m_data);

    const uint32_t allocatorChainSize = m_allocatorChain->Size();
    for (uint32_t i = 0; i < allocatorChainSize; ++i)
    {
        IcarianCore::Allocator* alloc = (*m_allocatorChain)[allocatorChainSize - i - 1];
        m_smallAllocator->Destroy(alloc);
    }

    m_smallAllocator->Destroy(m_allocatorChain);

    IcarianCore::MallocAllocator::Instance->Destroy(m_largeAllocator);
    IcarianCore::MallocAllocator::Instance->Destroy(m_smallAllocator);
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
        Instance = IcarianCore::MallocAllocator::Instance->Create<FileCache>(a_sizeMB, a_pipefileID);
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
        IcarianCore::MallocAllocator::Instance->Destroy(Instance);
        Instance = nullptr;
    }
}

static FileBuffer* GenerateFileBuffer(FileHandle* a_file, IcarianCore::Allocator* a_allocator)
{
    const uint64_t size = a_file->GetSize();

    FileBuffer* buffer = a_allocator->ZTAllocate<FileBuffer>();
    buffer->Size = size;
    buffer->Data = a_allocator->ZTAllocate<uint8_t>(size);
    if (a_file->Read(buffer->Data, size) != size)
    {
        IERROR("Failed to read FileHandle into FileBuffer");
    }
    buffer->TimePoint = std::chrono::high_resolution_clock::now();

    return buffer;
}

void FileCache::SubmitPipeRequest(IcarianCore::e_PipefileDataType a_type, const char* a_path, uint32_t a_size, uint32_t a_offset)
{
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    if (Instance == nullptr)
    {
        IERROR("Making pipe request with no FileCache");
    }

    if (Instance->m_data->CommandBuffer == NULL || Instance->m_data->DataBuffer == NULL)
    {
        IERROR("Making pipe request with no pipe");
    }

    // TODO: I should probably build a work queue so I can be smarter about awaiting
    if (a_path[0] == 0)
    {
        IERROR("Pipe request with empty string");
    }

    const char* slider = a_path;
    while (*slider != 0)
    {
        ++slider;
    }

    const uint32_t pathSize = (uint32_t)(slider - a_path);
    if (pathSize >= IcarianCore::PipefileHeader::MaxPathSize - 1)
    {
        IERROR("Pipe path size greater then max path size");
    }

    const std::unique_lock g = std::unique_lock(Instance->m_commandPipelock);

    while (Instance->m_data->CommandBuffer->State != IcarianCore::SharedMemoryBufferState_Write)
    {
        // The other side of the buffer is probably busy so just wait for it to be populated
        // This is mostly so the other process can get access to resources incase it is us hogging them
        std::this_thread::yield();
    }

    IVERIFY(Instance->m_data->CommandBuffer->Size >= sizeof(IcarianCore::PipefileHeader));
    IDEFER(Instance->m_data->CommandBuffer->State = IcarianCore::SharedMemoryBufferState_Read);

    IcarianCore::PipefileHeader* header = GetPipefileHeader(Instance->m_data->CommandBuffer);
    header->Type = a_type;
    header->Size = a_size;
    header->Offset = a_offset;
    header->Partial = false;
    memcpy(header->Path, a_path, pathSize);
    header->Path[pathSize] = 0;
#endif
}
e_PipeFileError FileCache::AwaitPipeData(IcarianCore::e_PipefileDataType a_type, const char* a_path, uint32_t* a_bufferSize, uint8_t** a_data)
{
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    return AwaitPipeData(a_type, a_path, 0, a_bufferSize, a_data);
#else
    return PipeFileError_Invalid;
#endif
}
e_PipeFileError FileCache::AwaitPipeData
(
    IcarianCore::e_PipefileDataType a_type,
    const char* a_path,
    uint32_t a_offset,
    uint32_t* a_bufferSize,
    uint8_t** a_data
)
{
    IVERIFY(a_bufferSize != nullptr);
    IVERIFY(a_data != nullptr);

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
    if (Instance == nullptr)
    {
        IERROR("Awaiting pipe data with no FileCache");
    }

    if (Instance->m_data->CommandBuffer == NULL || Instance->m_data->ReadBuffer == NULL)
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

            if (Instance->m_data->DataBuffer->State == IcarianCore::SharedMemoryBufferState_Read)
            {
                IVERIFY(Instance->m_data->DataBuffer->Size > sizeof(IcarianCore::PipefileHeader));

                IcarianCore::PipefileHeader* header = GetPipefileHeader(Instance->m_data->DataBuffer);

                if (header->Type == IcarianCore::PipefileDataType_Invalid)
                {
                    IVERIFY(header->Size == sizeof(IcarianCore::PipefileHeader));

                    volatile IcarianCore::PipefileHeader* invalidHeader = (IcarianCore::PipefileHeader*)GetDataSection(header);
                    if (invalidHeader->Type == a_type && invalidHeader->Offset == a_offset && strcmp(header->Path, a_path) == 0)
                    {
                        Instance->m_data->DataBuffer->State = IcarianCore::SharedMemoryBufferState_Write;

                        return PipeFileError_Invalid;
                    }
                }

                if (header->Type == a_type && header->Offset == a_offset && strcmp(header->Path, a_path) == 0)
                {
                    Instance->m_readLock.lock();

                    IDEFER(Instance->m_data->DataBuffer->State = IcarianCore::SharedMemoryBufferState_Write);

                    const uint32_t size = header->Size;
                    if (size > SharedBufferSize - sizeof(IcarianCore::PipefileHeader) - sizeof(IcarianCore::SharedMemoryBuffer))
                    {
                        IERROR("Pipe await overflow");
                    }

                    volatile uint8_t* data = GetDataSection(header);

                    *a_data = (uint8_t*)Instance->m_data->ReadBuffer;

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

bool FileCache::Exists(const char* a_str)
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_str, Instance->m_allocator);

    return Exists(str);
}
bool FileCache::Exists(const IcarianCore::COWU8String& a_str)
{
    if (Instance != nullptr)
    {
        const size_t protocolIndex = a_str.FindString("://");
        if (protocolIndex != uint32_t(-1))
        {
            constexpr uint32_t BufferSize = 16;
            if (protocolIndex >= (BufferSize - 1))
            {
                IERROR("Protocol buffer would overflow");
            }

            char buffer[BufferSize];
            memset(buffer, 0, BufferSize);

            for (uint32_t i = 0; i < protocolIndex; ++i)
            {
                buffer[i] = a_str[i];
            }

            switch (StringHash<uint32_t>(buffer))
            {
            case StringHash<uint32_t>("pipe"):
            {
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
                if (Instance->m_data->DataBuffer == NULL || Instance->m_data->CommandBuffer == NULL)
                {
                    return false;
                }

                const uint32_t length = a_str.Length();
                const uint32_t offset = protocolIndex + 3;
                const uint32_t size = length - offset;
                if (size >= IcarianCore::PipefileHeader::MaxPathSize)
                {
                    IERROR("Checking if file from pipe with path length greater then max path size");

                    return false;
                }

                char pathBuffer[IcarianCore::PipefileHeader::MaxPathSize];
                memset(pathBuffer, 0, IcarianCore::PipefileHeader::MaxPathSize);

                for (uint32_t i = 0; i < size; ++i)
                {
                    pathBuffer[i] = a_str[i + offset];
                }

                SubmitPipeRequest(IcarianCore::PipefileDataType_Exists, pathBuffer);

                uint8_t* data;
                uint32_t dataSize;
                const e_PipeFileError error = AwaitPipeData(IcarianCore::PipefileDataType_Exists, pathBuffer, &dataSize, &data);

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

    const char* cStr = a_str.CStr();
    return std::filesystem::exists(cStr);
}
bool FileCache::ExistsInCache(const char* a_str)
{
    if (Instance == nullptr)
    {
        return false;
    }

    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_str, Instance->m_allocator);

    return ExistsInCache(str);
}
bool FileCache::ExistsInCache(const IcarianCore::COWU8String& a_str)
{
    if (Instance == nullptr)
    {
        return false;
    }

    const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(Instance->m_lock);

    return Instance->m_data->Files.Exists(a_str);
}

void FileCache::PushFile(const char* a_path, const uint8_t* a_data, uint32_t a_size, bool a_pin)
{
    if (Instance == nullptr)
    {
        IWARN("Pushing file with no FileCache");

        return;
    }

    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, Instance->m_allocator);

    PushFile(str, a_data, a_size, a_pin);
}
void FileCache::PushFile(const IcarianCore::COWU8String& a_path, const uint8_t* a_data, uint32_t a_size, bool a_pin)
{
    if (Instance == nullptr)
    {
        IWARN("Pushing file with no FileCache");

        return;
    }

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(Instance->m_lock);

    if (Instance->m_data->Files.Exists(a_path))
    {
        FileBuffer* buffer = Instance->m_data->Files[a_path];
        while (buffer->Lock > 0)
        {
            std::this_thread::yield();
        }

        if (buffer->Size < a_size)
        {
            Instance->m_allocator->Free(buffer->Data);
            buffer->Data = Instance->m_allocator->TAllocate<uint8_t>(a_size);
        }

        buffer->Size = a_size;
        memcpy(buffer->Data, a_data, (size_t)a_size);
        buffer->TimePoint = std::chrono::high_resolution_clock::now();

        ITOGGLEBIT(a_pin, buffer->Flags, FileBuffer::PinnedBit);

        return;
    }

    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, Instance->m_allocator);

    FileBuffer* buffer = Instance->m_allocator->ZTAllocate<FileBuffer>();
    buffer->Size = a_size;
    buffer->Data = Instance->m_allocator->TAllocate<uint8_t>();
    memcpy(buffer->Data, a_data, (size_t)a_size);
    buffer->TimePoint = std::chrono::high_resolution_clock::now();

    if (a_pin)
    {
        ISETBIT(buffer->Flags, FileBuffer::PinnedBit);
    }

    Instance->m_data->Files.Push(str, buffer);
}

void FileCache::Update()
{
    if (Instance == nullptr)
    {
        return;
    }

    const uint64_t allocated = Instance->m_trackerAllocator->GetMemoryUsage();
    Profiler::PushMemoryFrame(ProfilerMemoryFrame_FileCache, allocated);

    Instance->m_data->UpdateFrame = (Instance->m_data->UpdateFrame + 1) % 4;
    // Want to clear it if we are over half full but do not need to do it regularly
    if (Instance->m_data->UpdateFrame != 0)
    {
        return;
    }

    const uint64_t halfSize = Instance->m_data->Size >> 1;
    if (allocated < halfSize)
    {
        return;
    }

    const std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(Instance->m_lock);

    const IcarianCore::Array<IcarianCore::COWU8String> keyArray = Instance->m_data->Files.GetKeys(Instance->m_allocator);
    if (keyArray.Empty())
    {
        return;
    }

    const IcarianCore::Array<FileBuffer*> valueArray = Instance->m_data->Files.GetValues(Instance->m_allocator);
    IVERIFY(keyArray.Size() == valueArray.Size());

    const uint32_t size = keyArray.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        FileBuffer* buffer = valueArray[i];
        if (IISBITSET(buffer->Flags, FileBuffer::PinnedBit))
        {
            continue;
        }

        if (buffer->Lock != 0)
        {
            continue;
        }

        const std::chrono::duration timePassed = std::chrono::duration(now - buffer->TimePoint);
        if (timePassed <= std::chrono::seconds(1))
        {
            continue;
        }

        Instance->m_data->Files.Erase(keyArray[i]);

        Instance->m_allocator->Free(buffer->Data);
        Instance->m_allocator->Destroy(buffer);
    }
}

FileHandle* FileCache::LoadFile(const char* a_path)
{
    const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, Instance->m_allocator);

    return LoadFile(str);
}
FileHandle* FileCache::LoadFile(const IcarianCore::COWU8String& a_path)
{
    if (Instance == nullptr)
    {
        return ReadFileHandle::OpenFile(a_path);
    }

    {
        const IcarianCore::SharedThreadGuard g = IcarianCore::SharedThreadGuard(Instance->m_lock);

        if (Instance->m_data->Files.Exists(a_path))
        {
            FileBuffer* buffer = Instance->m_data->Files[a_path];

            return IcarianCore::MallocAllocator::Instance->Create<CacheFileHandle>(buffer);
        }
    }

    const IcarianCore::ThreadGuard g = IcarianCore::ThreadGuard(Instance->m_lock);

    FileHandle* handle = ILAMBDA(
    {
        const size_t protocolIndex = a_path.FindString("://");
        if (protocolIndex != uint32_t(-1))
        {
            constexpr uint32_t BufferSize = 16;
            if (protocolIndex >= (BufferSize - 1))
            {
                IERROR("Protocol buffer would overflow");
            }

            char buffer[BufferSize];
            memset(buffer, 0, BufferSize);

            for (uint32_t i = 0; i < protocolIndex; ++i)
            {
                buffer[i] = a_path[i];
            }

            switch (StringHash<uint32_t>(buffer))
            {
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
            case StringHash<uint32_t>("pipe"):
            {
                const uint32_t length = a_path.Length();
                const uint32_t offset = protocolIndex + 3;
                const uint32_t size = length - offset;

                if (size >= IcarianCore::PipefileHeader::MaxPathSize)
                {
                    IERROR("Reading file from pipe with path length greater then max path size");

                    ILRETURN (FileHandle*)nullptr;
                }

                char pathBuffer[IcarianCore::PipefileHeader::MaxPathSize];
                memset(pathBuffer, 0, IcarianCore::PipefileHeader::MaxPathSize);

                for (uint32_t i = 0; i < size; ++i)
                {
                    pathBuffer[i] = a_path[i + offset];
                }

                ILRETURN (FileHandle*)IcarianCore::MallocAllocator::Instance->Create<PipeFileHandle>(pathBuffer);
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

    const uint64_t maxSize = Instance->m_data->Size >> 3;
    const uint64_t size = handle->GetSize();
    if (size >= maxSize)
    {
        return handle;
    }

    const uint64_t allocated = Instance->m_trackerAllocator->GetMemoryUsage();

    const uint32_t overheadSize = size + 256;
    if (overheadSize < Instance->m_data->Size - allocated)
    {
        IDEFER(IcarianCore::MallocAllocator::Instance->Destroy(handle));

        FileBuffer* buffer = GenerateFileBuffer(handle, Instance->m_allocator);

        const IcarianCore::COWU8String str = IcarianCore::COWU8String(a_path, Instance->m_allocator);
        Instance->m_data->Files.Push(str, buffer);

        return IcarianCore::MallocAllocator::Instance->Create<CacheFileHandle>(buffer);
    }

    const uint64_t offsetSize = ILAMBDA(
    {
        if (allocated > Instance->m_data->Size)
        {
            ILRETURN allocated - Instance->m_data->Size;
        }

        ILRETURN uint64_t(0);
    });

    const uint64_t finalSize = offsetSize + overheadSize;

    const IcarianCore::Array<IcarianCore::COWU8String> keyArray = Instance->m_data->Files.GetKeys(Instance->m_allocator);
    const IcarianCore::Array<FileBuffer*> valueArray = Instance->m_data->Files.GetValues(Instance->m_allocator);

    IVERIFY(keyArray.Size() == valueArray.Size());

    IcarianCore::COWU8String key = IcarianCore::COWU8String(Instance->m_allocator);
    FileBuffer* b = nullptr;
    const uint32_t keySize = keyArray.Size();
    for (uint32_t i = 0; i < keySize; ++i)
    {
        FileBuffer* buffer = valueArray[i];
        if (IISBITSET(buffer->Flags, FileBuffer::PinnedBit))
        {
            continue;
        }

        if (buffer->Size < finalSize)
        {
            continue;
        }

        if (buffer->Lock != 0)
        {
            continue;
        }

        if (b == nullptr)
        {
            key = keyArray[i];
            b = buffer;

            continue;
        }

        if (buffer->TimePoint > b->TimePoint)
        {
            continue;
        }

        key = keyArray[i];
        b = buffer;
    }

    if (b == nullptr)
    {
        return handle;
    }

    IDEFER(IcarianCore::MallocAllocator::Instance->Destroy(handle));

    Instance->m_data->Files.Erase(key);
    Instance->m_allocator->Free(b->Data);
    Instance->m_allocator->Destroy(b);

    b = GenerateFileBuffer(handle, Instance->m_allocator);

    Instance->m_data->Files.Push(a_path, b);

    return IcarianCore::MallocAllocator::Instance->Create<CacheFileHandle>(b);
}

// MIT License
//
// Copyright (c) 2026 River Govers
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
