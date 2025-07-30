// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Core/SharedMemoryBuffer.h"
#include "Core/Pipefile.h"
#include "DataTypes/SpinLock.h"
#include "FileHandles/CacheFileHandle.h"

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
    SharedSpinLock                               m_lock;
    uint64_t                                     m_size;
    uint64_t                                     m_allocated;
    uint32_t                                     m_updateFrame;

    // Use string as compilers seem to be hit or miss as to path as a key
    std::unordered_map<std::string, FileBuffer*> m_files;

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE
#ifndef WIN32
    static constexpr char CommandBufferName[] = "IcarianEditorAssetCommand";
    static constexpr char DataBufferName[] = "IcarianEditorAssetData";
#endif

    static constexpr uint32_t SharedBufferSize = 10 << 10;

    uint32_t                                     m_pipefileID;

    // May be a while so just use a mutex over a spinlock
    std::mutex                                   m_commandPipelock;
    std::mutex                                   m_dataPipelock;
    std::mutex                                   m_readLock;

    IcarianCore::SharedMemoryBuffer*             m_commandBuffer;
    IcarianCore::SharedMemoryBuffer*             m_dataBuffer;

    void*                                        m_readBuffer;
#endif

    FileCache(uint32_t a_sizeMiB, uint32_t a_pipefileID);

    FileHandle* GenerateFileHandle(const std::string& a_path, FILE* a_file, uint64_t a_size);

protected:

public:
    ~FileCache();

    static void Init(uint32_t a_sizeMiB, uint32_t a_pipefileID);
    static void Destroy();

    // I am not a fan of this there is a very bad flaw that can result in getting the data of an unrelated request for now but just ignoring it
    // Either way this has awful code smell and not happy with it
    // TOOD: Fix this may need to unify this into a single function with blocking depending on shit
    static void SubmitPipeRequest(IcarianCore::e_PipefileDataType a_type, const std::string_view& a_path, uint32_t a_size = 0, uint32_t a_offset = 0);
    static e_PipeFileError AwaitPipeData(IcarianCore::e_PipefileDataType a_type, const std::string_view& a_path, uint32_t* a_bufferSize, uint8_t** a_data);
    static e_PipeFileError AwaitPipeData(IcarianCore::e_PipefileDataType a_type, const std::string_view& a_path, uint32_t a_offset, uint32_t* a_bufferSize, uint8_t** a_data);
    static void FreePipeData();

    static bool Exists(const std::string_view& a_str);
    static bool ExistsInCache(const std::string_view& a_str);

    static void Update();

    static void PushFile(const std::string_view& a_str, uint8_t* a_data, uint32_t a_size, bool a_pin);
    static FileHandle* LoadCachedFile(const std::string_view& a_str);

    static FileHandle* LoadFile(const std::string_view& a_path);
};

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
