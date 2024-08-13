// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/Video/VideoManagerBindings.h"

#include "IcarianError.h"
#include "Rendering/Video/VideoClip.h"
#include "Rendering/Video/VideoManager.h"
#include "Runtime/RuntimeManager.h"

static VideoManagerBindings* Instance = nullptr;

#include "EngineVideoClipInterop.h"

ENGINE_VIDEOCLIP_EXPORT_TABLE(RUNTIME_FUNCTION_DEFINITION);

VideoManagerBindings::VideoManagerBindings(VideoManager* a_manager)
{
    Instance = this;

    m_manager = a_manager;

    ENGINE_VIDEOCLIP_EXPORT_TABLE(RUNTIME_FUNCTION_ATTACH);
}
VideoManagerBindings::~VideoManagerBindings()
{

}

uint32_t VideoManagerBindings::GenerateVideoClipFromFile(const std::filesystem::path& a_path) const
{
    VideoClip* clip = new VideoClip(a_path);

    uint8_t* dat;
    uint32_t size;
    if (clip->GetVideoClipData(0, nullptr, nullptr, &dat, &size))
    {
        delete[] dat;
    }

    return m_manager->m_clips.PushVal(clip);
}
void VideoManagerBindings::DestroyVideoClip(uint32_t a_addr) const
{
    IVERIFY(a_addr < m_manager->m_clips.Size());
    IVERIFY(m_manager->m_clips.Exists(a_addr));

    const VideoClip* clip = m_manager->m_clips[a_addr];
    IDEFER(delete clip);
    m_manager->m_clips.Erase(a_addr);
}

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