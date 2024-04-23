#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <shared_mutex>
#include <unordered_map>

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
    virtual void Seek(uint64_t a_offset) = 0;
    virtual void Ignore(uint64_t a_size) = 0;
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
    virtual void Seek(uint64_t a_offset);
    virtual void Ignore(uint64_t a_size);
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
    virtual void Seek(uint64_t a_offset);
    virtual void Ignore(uint64_t a_size);
    virtual bool EndOfFile() const;
};

// RAM is incredibly slow but spinning rust is much slower then RAM,
// therefore use RAM to reduce file access if at all possible
class FileCache
{
private:
    std::shared_mutex                                      m_mutex;
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