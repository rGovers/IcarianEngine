#include "Rendering/Video/VideoManager.h"

#include "IcarianError.h"
#include "Rendering/Video/VideoManagerBindings.h"

static VideoManager* Instance = nullptr;

VideoManager::VideoManager()
{
    m_bindings = new VideoManagerBindings(this);
}
VideoManager::~VideoManager()
{
    delete m_bindings;
}

void VideoManager::Init()
{
    if (Instance == nullptr)
    {
        Instance = new VideoManager();
    }
}
void VideoManager::Destroy()
{
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
}

VideoClip* VideoManager::GetVideoClip(uint32_t a_addr)
{
    IVERIFY(a_addr < Instance->m_clips.Size());
    IVERIFY(Instance->m_clips.Exists(a_addr));

    return Instance->m_clips[a_addr];
}