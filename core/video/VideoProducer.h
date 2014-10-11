
#ifndef VIDEOPRODUCER_H
#define VIDEOPRODUCER_H

#ifdef WIN32
#include <d3d9.h>
#endif

/**
 * This interface is implemented by any piece of hardware that renders graphic
 * output.
 */
class VideoProducer
{

    public:
        /**
         * Tells the video producer to render its output.  It should *not* flip
         * the output onto the screen as that will be done by the video bus
         * after all video producers have rendered their output to the back
         * buffer.
         *
         * This function will be called once each time the Emulator indicates
         * that it has entered vertical blank.
         */

#ifdef WIN32
        virtual void setVideoOutputDevice(IDirect3DDevice9* vod) = 0;
#endif
        virtual void render() = 0;

};

#endif
