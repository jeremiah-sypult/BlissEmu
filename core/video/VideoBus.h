
#ifndef VIDEOBUS_H
#define VIDEOBUS_H

#include <d3d9.h>

#include "VideoProducer.h"

const INT32 MAX_VIDEO_PRODUCERS = 10;

class VideoBus
{

    public:
        VideoBus();
        void addVideoProducer(VideoProducer* ic);
        void removeVideoProducer(VideoProducer* ic);
        void removeAll();

        void init(IDirect3DDevice9* id);
        void render();
        void release();

    private:
        IDirect3DDevice9*     videoOutputDevice;
        VideoProducer*        videoProducers[MAX_VIDEO_PRODUCERS];
        UINT32                videoProducerCount;

};

#endif
