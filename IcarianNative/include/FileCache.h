// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <mutex>

#include "Core/SharedMemoryBuffer.h"
#include "Core/Pipefile.h"
#include "DataTypes/COWString.h"
#include "DataTypes/Dictionary.h"
#include "DataTypes/SpinLock.h"
#include "FileHandles/CacheFileHandle.h"

class BlockAllocator;
class TrackerAllocator;

enum e_PipeFileError
{
    PipeFileError_Success,
    PipeFileError_Partial,
    PipeFileError_Invalid
};

// RAM is incredibly slow but spinning rust is much slower then RAM,
// therefore use RAM to reduce file access if at all possible
class FileCache
{
private:
#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
#ifndef WIN32
    static constexpr char CommandBufferName[] = "IcarianEditorAssetCommand";
    static constexpr char DataBufferName[] = "IcarianEditorAssetData";
#endif

    static constexpr uint32_t SharedBufferSize = 10 << 10;
#endif

    static constexpr uint32_t SmallAllocatorSize = 8 << 10;
    static constexpr uint32_t LargeAllocatorSize = 8 << 20;

    struct ClassData
    {
        uint64_t                             Size;
        uint32_t                             UpdateFrame;

        // Use string as compilers seem to be hit or miss as to path as a key
        Dictionary<COWU8String, FileBuffer*> Files;

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
        uint32_t                             PipefileID;

        IcarianCore::SharedMemoryBuffer*     CommandBuffer;
        IcarianCore::SharedMemoryBuffer*     DataBuffer;

        void*                                ReadBuffer;
#endif
    };

    BlockAllocator*                      m_smallAllocator;
    BlockAllocator*                      m_largeAllocator;

    Allocator*                           m_allocator;
    TrackerAllocator*                    m_trackerAllocator;

    Array<Allocator*>*                   m_allocatorChain;

    ClassData*                           m_data;

    SharedSpinLock                       m_lock;

    // May be a while so just use a mutex over a spinlock
    std::mutex                           m_commandPipelock;
    std::mutex                           m_dataPipelock;
    std::mutex                           m_readLock;

    FileHandle* GenerateFileHandle(const COWU8String& a_path, FILE* a_file, uint64_t a_size);

protected:

public:
    FileCache(uint32_t a_sizeMiB, uint32_t a_pipefileID);
    ~FileCache();

    static void Init(uint32_t a_sizeMiB, uint32_t a_pipefileID);
    static void Destroy();

    // I am not a fan of this there is a very bad flaw that can result in getting the data of an unrelated request for now but just ignoring it
    // Either way this has awful code smell and not happy with it
    // TOOD: Fix this may need to unify this into a single function with blocking depending on shit
    static void SubmitPipeRequest(IcarianCore::e_PipefileDataType a_type, const char* a_path, uint32_t a_size = 0, uint32_t a_offset = 0);
    static e_PipeFileError AwaitPipeData(IcarianCore::e_PipefileDataType a_type, const char* a_path, uint32_t* a_bufferSize, uint8_t** a_data);
    static e_PipeFileError AwaitPipeData
    (
        IcarianCore::e_PipefileDataType a_type,
        const char* a_path,
        uint32_t a_offset,
        uint32_t* a_bufferSize,
        uint8_t** a_data
    );
    static void FreePipeData();

    static bool Exists(const char* a_path);
    static bool Exists(const COWU8String& a_path);
    static bool ExistsInCache(const char* a_path);
    static bool ExistsInCache(const COWU8String& a_path);

    static void Update();

    static void PushFile(const char* a_path, const uint8_t* a_data, uint32_t a_size, bool a_pin);
    static void PushFile(const COWU8String& a_path, const uint8_t* a_data, uint32_t a_size, bool a_pin);
    static FileHandle* LoadCachedFile(const char* a_path);
    static FileHandle* LoadCachedFile(const COWU8String& a_path);

    static FileHandle* LoadFile(const char* a_path);
    static FileHandle* LoadFile(const COWU8String& a_path);
};

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
