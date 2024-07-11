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