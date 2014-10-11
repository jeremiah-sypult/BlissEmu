
#include "VideoBus.h"

VideoBus::VideoBus()
{
    videoOutputDevice = NULL;
    videoProducerCount = 0;
}

void VideoBus::addVideoProducer(VideoProducer* p)
{
    videoProducers[videoProducerCount] = p;
    videoProducers[videoProducerCount]->setVideoOutputDevice(videoOutputDevice);
    videoProducerCount++;
}

void VideoBus::removeVideoProducer(VideoProducer* p)
{
    for (UINT32 i = 0; i < videoProducerCount; i++) {
        if (videoProducers[i] == p) {
		    videoProducers[i]->setVideoOutputDevice(NULL);
            for (UINT32 j = i; j < (videoProducerCount-1); j++)
                videoProducers[j] = videoProducers[j+1];
            videoProducerCount--;
            return;
        }
    }
}

void VideoBus::removeAll()
{
    while (videoProducerCount)
        removeVideoProducer(videoProducers[0]);
}

void VideoBus::init(IDirect3DDevice9* vod)
{
    this->videoOutputDevice = vod;
    for (UINT32 i = 0; i < videoProducerCount; i++)
        videoProducers[i]->setVideoOutputDevice(vod);
}

void VideoBus::render()
{
    //tell each of the video producers that they can now output their
    //video contents onto the video device
    for (UINT32 i = 0; i < videoProducerCount; i++)
        videoProducers[i]->render();
}

void VideoBus::release()
{
    for (UINT32 i = 0; i < videoProducerCount; i++)
        videoProducers[i]->setVideoOutputDevice(NULL);
    this->videoOutputDevice = NULL;
}
