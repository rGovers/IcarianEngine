#pragma once

#include "DataTypes/TNCArray.h"

class VideoClip;
class VideoManagerBindings;

class VideoManager
{
private:
    friend class VideoManagerBindings;

    VideoManagerBindings* m_bindings;
    TNCArray<VideoClip*>  m_clips;

    VideoManager();
    
protected:

public:
    ~VideoManager();

    static void Init();
    static void Destroy();

    static VideoClip* GetVideoClip(uint32_t a_addr);
};
