// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include "Endian.h"

namespace IcarianCore
{
    constexpr uint32_t CRC32(const void* a_data, const uint64_t a_size)
    {
        constexpr struct TableStruct
        {   
            uint8_t Data[256];

            constexpr TableStruct() : Data()
            {
                if constexpr (IsBigEndian())
                {
                    uint32_t crc = 0x8000;
                    for (uint32_t i = 1; i < sizeof(Data); i = i << 1)
                    {
                        if (crc & 0x8000)
                        {
                            crc = (crc << 1) ^ 0x1021;
                        }
                        else
                        {
                            crc <<= 1;
                        }

                        for (uint32_t j = 0; j < i; ++j)
                        {
                            Data[i + j] = crc ^ Data[j];
                        }
                    }
                }
                else
                {
                    uint32_t crc = 1;
                    for (uint32_t i = 128; i > 0; i = i >> 1)
                    {
                        if (crc & 1)
                        {
                            crc = (crc >> 1) ^ 0x8408;
                        }
                        else
                        {
                            crc >>= 1;
                        }

                        for (uint32_t j = 0; j < 255; j += i * 2)
                        {
                            Data[i + j] = crc ^ Data[j];
                        }
                    }
                }   
            }
        } Table;

        uint32_t crc32 = 0xFFFFFFFF;

        const char* start = (char*)a_data;
        const char* end = start + a_size;
        for (const char* dPtr = start; dPtr < end; ++dPtr)
        {
            const uint8_t index = (crc32 ^ *dPtr) & 0xFF;
            crc32 = (crc32 >> 8) ^ Table.Data[index];
        }

        return crc32 ^ 0xFFFFFFFF;
    }
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