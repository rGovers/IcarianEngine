// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "FileHandles/ReadFileHandle.h"

#include <filesystem>

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/IcarianError.h"

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

ReadFileHandle* ReadFileHandle::OpenFile(const IcarianCore::COWU8String& a_path)
{
    IERRBLOCK;

    const char* cStr = a_path.CStr();

    IERRCHECKRET(std::filesystem::exists(cStr), nullptr);

    FILE* fp = fopen(cStr, "rb");
    IERRCHECKRET(fp != NULL, nullptr);
    IERRDEFER(fclose(fp));

    fseek(fp, 0L, SEEK_END);
    const long size = ftell(fp);
    rewind(fp);

    IERRCHECKRET(size >= 0, nullptr);

    return IcarianCore::MallocAllocator::Instance->Create<ReadFileHandle>(fp, (uint64_t)size);
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
