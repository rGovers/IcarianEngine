// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#ifdef ICARIANNATIVE_ENABLE_PIPEFILE

#include "FileHandle.h"

#include "Core/Pipefile.h"

class PipeFileHandle : public FileHandle
{
private:
    uint32_t m_size;
    uint32_t m_offset;

    char     m_path[IcarianCore::PipefileHeader::MaxPathSize];

protected:

public:
    PipeFileHandle(const char* a_path);
    virtual ~PipeFileHandle();

    virtual uint64_t GetSize() const;
    virtual uint64_t GetOffset() const;
    virtual uint64_t Read(void* a_data, uint64_t a_size);
    virtual bool Seek(uint64_t a_offset);
    virtual bool Ignore(uint64_t a_size);
    virtual bool EndOfFile() const;
};

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
