// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <unordered_map>

#include "DataTypes/SpinLock.h"

struct FileBuffer
{
    uint64_t Size;
    void* Data;
    std::chrono::high_resolution_clock::time_point TimePoint;
    std::atomic<uint32_t> Lock;
};

class FileHandle
{
private:

protected:

public:
    virtual ~FileHandle() { }

    virtual uint64_t GetSize() const = 0;
    virtual uint64_t GetOffset() const = 0;
    virtual uint64_t Read(void* a_data, uint64_t a_size) = 0;
    virtual bool Seek(uint64_t a_offset) = 0;
    virtual bool Ignore(uint64_t a_size) = 0;
    virtual bool EndOfFile() const = 0;
};

class CacheFileHandle : public FileHandle
{
private:
    FileBuffer* m_buffer;
    uint64_t    m_offset;

protected:

public:
    CacheFileHandle(FileBuffer* a_buffer);
    virtual ~CacheFileHandle();

    virtual uint64_t GetSize() const;
    virtual uint64_t GetOffset() const;
    virtual uint64_t Read(void* a_data, uint64_t a_size);
    virtual bool Seek(uint64_t a_offset);
    virtual bool Ignore(uint64_t a_size);
    virtual bool EndOfFile() const;
};

class ReadFileHandle : public FileHandle
{
private:
    FILE*    m_file;
    uint64_t m_size;

protected:

public:
    ReadFileHandle(FILE* a_file, uint64_t a_size);
    virtual ~ReadFileHandle();

    virtual uint64_t GetSize() const;
    virtual uint64_t GetOffset() const;
    virtual uint64_t Read(void* a_data, uint64_t a_size);
    virtual bool Seek(uint64_t a_offset);
    virtual bool Ignore(uint64_t a_size);
    virtual bool EndOfFile() const;
};

// RAM is incredibly slow but spinning rust is much slower then RAM,
// therefore use RAM to reduce file access if at all possible
class FileCache
{
private:
    SharedSpinLock                                         m_lock;
    uint64_t                                               m_size;
    uint64_t                                               m_allocated;
    uint32_t                                               m_updateFrame;

    std::unordered_map<std::filesystem::path, FileBuffer*> m_files;

    FileCache(uint32_t a_sizeMiB);

    FileHandle* GenerateFileHandle(const std::filesystem::path& a_path, FILE* a_file, uint64_t a_size);

protected:

public:
    ~FileCache();

    static void Init(uint32_t a_sizeMiB);
    static void Destroy();

    static void Update();

    static void PreLoad(const std::filesystem::path& a_path);
    static FileHandle* LoadFile(const std::filesystem::path& a_path);
};

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