// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include <cstdint>
#include <stb_truetype.h>

#include "Core/DataTypes/Array.h"
#include "Core/DataTypes/COWString.h"

#include "EngineModelInteropStructures.h"

class Font
{
private:
    IcarianCore::Allocator* m_allocator;

    stbtt_fontinfo          m_fontInfo;
    // Forgot that stbb_fontinfo does not own the data or copy it, so we need to keep it around
    uint8_t*                m_data;

protected:

public:
    Font(uint8_t* a_data, IcarianCore::Allocator* a_allocator);
    ~Font();

    static bool LoadFont(Font* a_font, const IcarianCore::COWU8String& a_path, IcarianCore::Allocator* a_allocator);

    uint8_t* StringToTexture
    (
        const IcarianCore::COWU32String& a_string,
        float a_fontSize,
        uint32_t a_width,
        uint32_t a_height,
        IcarianCore::Allocator* a_allocator,
        IcarianCore::Allocator* a_tempAllocator
    ) const;
    void StringToModel
    (
        const IcarianCore::COWU32String& a_string,
        float a_fontSize,
        float a_scale,
        float a_depth,
        IcarianCore::Array<Vertex>* a_vertices,
        IcarianCore::Array<uint32_t>* a_indices,
        float* a_radius,
        IcarianCore::Allocator* a_allocator,
        IcarianCore::Allocator* a_tempAllocator
    ) const;
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
