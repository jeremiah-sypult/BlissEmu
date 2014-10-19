// 
// BlissAudioVideo.h
//

#include <d3d9.h>
#include <mmreg.h>
#include <dsound.h>
#include "core/Emulator.h"

class AudioMixerWinDS : public AudioMixer
{
public:
	AudioMixerWinDS(IDirectSoundBuffer8* buffer);

	void resetProcessor();
	void flushAudio();
	void init(UINT32 sampleRate);
	void release();

	IDirectSoundBuffer8*		outputBuffer;
	UINT32					outputBufferSize;
	UINT32					outputBufferWritePosition;
};

class VideoBusWinD3D : public VideoBus
{
public:
	VideoBusWinD3D(IDirect3DDevice9* device);

	void init(UINT32 width, UINT32 height);
	void release();
	void render();

	IDirect3DDevice9*       videoOutputDevice;
	IDirect3DTexture9*      combinedTexture;
	IDirect3DVertexBuffer9* vertexBuffer;
	D3DLOCKED_RECT          combinedBufferLock;
};
