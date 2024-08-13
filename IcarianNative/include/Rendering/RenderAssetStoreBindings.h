// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <filesystem>

class RenderAssetStore;

class RenderAssetStoreBindings
{
private:
    RenderAssetStore* m_store;

protected:

public:
    RenderAssetStoreBindings(RenderAssetStore* a_store);
    ~RenderAssetStoreBindings();

    uint32_t GenerateFont(const std::filesystem::path& a_path) const;
    void DestroyFont(uint32_t a_addr) const;

    uint32_t GenerateModelFromString(uint32_t a_addr, const std::u32string_view& a_str, float a_fontSize, float a_scale, float a_depth) const;

    uint32_t GenerateModel(const std::filesystem::path& a_path, uint32_t a_index) const;
    uint32_t GenerateSkinnedModel(const std::filesystem::path& a_path, uint32_t a_index) const;

    uint32_t GenerateTexture(const std::filesystem::path& a_path) const;
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