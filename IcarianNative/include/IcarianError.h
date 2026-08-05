// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/DataTypes/Allocators/MallocAllocator.h"
#include "Core/DataTypes/COWString.h"
#include "Logger.h"

#define IERRSTRR(v) #v
#define IERRSTR(v) IERRSTRR(v)

#define IWARN(msg) Logger::Warning(IcarianCore::COWU8String("IWARN: ", IcarianCore::MallocAllocator::Instance) + (msg))
#define IERROR(msg) IcarianError(IcarianCore::COWU8String("IERROR: ", IcarianCore::MallocAllocator::Instance) + (msg) + ": " IERRSTR(__FILE__) "," IERRSTR(__LINE__))
#ifdef DEBUG
#define IVERIFY(val) do { if (!(val)) { IERROR(#val); } } while (0)
#else
#define IVERIFY(val) void(0)
#endif

void IcarianError(const IcarianCore::COWU8String& a_msg);

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
