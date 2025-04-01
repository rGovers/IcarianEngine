// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/Video/VideoClip.h"

#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <minimp4.h>

#include "Core/IcarianDefer.h"
#include "Core/IcarianError.h"
#include "FileCache.h"
#include "IcarianError.h"
#include "Rendering/Video/VideoInfo/H264VideoInfo.h"

static int ReadCallback(int64_t a_offset, void* a_buffer, size_t a_size, void* a_token)
{
    if (a_offset < 0)
    {
        return -1;
    }

    FileHandle* handle = (FileHandle*)a_token;

    const uint64_t fileOffset = handle->GetOffset();
    if ((uint64_t)a_offset != fileOffset)
    {
        handle->Ignore(a_offset - fileOffset);
    }

    return handle->Read(a_buffer, a_size) != a_size;
}

VideoClip::VideoClip(const std::filesystem::path& a_path)
{
    IERRBLOCK;

    m_path = a_path;
    m_videoInfo = nullptr;

    m_time = 0.0;

    m_updateMode = VideoUpdateMode_Video;

    FileHandle* handle = FileCache::LoadFile(a_path);
    IERRCHECK(handle != nullptr);
    IDEFER(delete handle);

    MP4D_demux_t demux = { };
    IERRCHECK(MP4D_open(&demux, ReadCallback, handle, (int64_t)handle->GetSize()));
    IDEFER(MP4D_close(&demux));

    for (unsigned int i = 0; i < demux.track_count; ++i)
    {
        const MP4D_track_t& track = demux.track[i];

        switch (track.handler_type)
        {
        case MP4D_HANDLER_TYPE_VIDE:
        {
            switch (track.object_type_indication) 
            {
            case MP4_OBJECT_TYPE_AVC:
            {
                IVERIFY(m_videoInfo == nullptr);

                m_videoInfo = new H264VideoInfo(demux, track, i, handle);
                IERRDEFER(
                {
                    delete m_videoInfo;
                    m_videoInfo = nullptr;
                });
                IERRCHECK(m_videoInfo->IsValid());

                break;
            }
            default:
            {
                IERROR("Unsupported Video Profile");

                break;
            }
            }

            break;
        }
        case MP4D_HANDLER_TYPE_SOUN:
        {
            // TODO: Implement me!

            break;
        }
        }
    }
}
VideoClip::~VideoClip()
{

}

bool VideoClip::GetVideoClipData(uint32_t a_startIndex, uint32_t a_endIndex, uint32_t a_alignment, uint8_t** a_data, uint32_t* a_size) const
{
    IERRBLOCK;

    IVERIFY(a_data != nullptr);
    IVERIFY(a_size != nullptr);

    IERRCHECKRET(m_videoInfo != nullptr, false);

    FileHandle* handle = FileCache::LoadFile(m_path);
    IERRCHECKRET(handle != nullptr, false);
    IDEFER(delete handle);

    return m_videoInfo->GetVideoClipData(handle, a_startIndex, a_endIndex, a_alignment, a_data, a_size);
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