#pragma once

#include <cstdint>
#include <filesystem>

class VideoManager;

class VideoManagerBindings
{
private:
    VideoManager* m_manager;

protected:

public:
    VideoManagerBindings(VideoManager* a_manager);
    ~VideoManagerBindings();

    uint32_t GenerateVideoClipFromFile(const std::filesystem::path& a_path) const;
    void DestroyVideoClip(uint32_t a_addr) const;
};