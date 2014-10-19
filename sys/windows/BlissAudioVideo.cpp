//
// BlissAudioVideo.cpp
//

#include "BlissAudioVideo.h"

//
// audio implementation
//

AudioMixerWinDS::AudioMixerWinDS(IDirectSoundBuffer8* buffer)
  : outputBuffer(NULL),
    outputBufferSize(0),
    outputBufferWritePosition(0)
{
	this->outputBuffer = buffer;
	DSBCAPS caps;
	ZeroMemory(&caps, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	buffer->GetCaps(&caps);
	outputBufferWritePosition = 0;
	outputBufferSize = caps.dwBufferBytes;
	WAVEFORMATEX wfx;
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	buffer->GetFormat(&wfx, sizeof(WAVEFORMATEX), NULL);
}

void AudioMixerWinDS::resetProcessor()
{
	if (outputBuffer) {
		DWORD bufSize1, bufSize2;
		void* buf1; void* buf2;
		outputBuffer->Lock(outputBufferWritePosition, sampleSize, &buf1, &bufSize1, &buf2, &bufSize2, 0);
		memset(buf1, 0, bufSize1);
		memset(buf2, 0, bufSize2);
		outputBuffer->Unlock(buf1, bufSize1, buf2, bufSize2);

		outputBufferWritePosition = 0;
	}

	// clears the emulator side of the audio mixer
	AudioMixer::resetProcessor();
}


void AudioMixerWinDS::init(UINT32 sampleRate)
{
	release();

	outputBufferWritePosition = 0;

	AudioMixer::init(sampleRate);
}

void AudioMixerWinDS::release()
{
	AudioMixer::release();
}

void AudioMixerWinDS::flushAudio()
{
	DWORD currentPos;
	size_t sampleCountSize = sampleCount * sizeof(INT16);
	UINT32 end = (outputBufferWritePosition + sampleCountSize) % outputBufferSize;

	if (sampleCount < this->sampleSize) {
		return;
	}

	if (end < outputBufferWritePosition) {
		do {
			outputBuffer->GetCurrentPosition(&currentPos, NULL);
		} while (currentPos >= outputBufferWritePosition || currentPos < end);
	}
	else {
		do {
			outputBuffer->GetCurrentPosition(&currentPos, NULL);
		} while (currentPos >= outputBufferWritePosition && currentPos < end);
	}
	DWORD bufSize1, bufSize2;
	void* buf1; void* buf2;
	outputBuffer->Lock(outputBufferWritePosition, sampleCountSize, &buf1, &bufSize1, &buf2, &bufSize2, 0);
	memcpy(buf1, sampleBuffer, bufSize1);
	memcpy(buf2, sampleBuffer+bufSize1, bufSize2);
	outputBuffer->Unlock(buf1, bufSize1, buf2, bufSize2);
	outputBufferWritePosition = end;

	// clear the sample count
	AudioMixer::flushAudio();
}

//
// video implementation
//

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_TEX1)
struct CUSTOMVERTEX
{
	FLOAT  x, y, z, rhw; // The position
	FLOAT  tu, tv;       // The texture coordinates
};

VideoBusWinD3D::VideoBusWinD3D(IDirect3DDevice9* device)
  : videoOutputDevice(NULL),
    combinedTexture(NULL),
    vertexBuffer(NULL)
{
	videoOutputDevice = device;
}

void VideoBusWinD3D::init(UINT32 width, UINT32 height)
{
	release();

	if (this->videoOutputDevice != NULL) {
		FLOAT halfTexelWidth = (1.0 / width) * 0.5;
		FLOAT halfTexelHeight = (1.0 / height) * 0.5;

		videoOutputDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
		videoOutputDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);

		//obtain the surface we desire
		videoOutputDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8,
										 D3DPOOL_DEFAULT, &combinedTexture, NULL);

		combinedTexture->LockRect(0, &combinedBufferLock, NULL, D3DLOCK_DISCARD |  D3DLOCK_NOSYSLOCK);
		memset(combinedBufferLock.pBits, 0xFFFFFFFF, sizeof(UINT32) * width * height);
		combinedTexture->UnlockRect(0);

		//create our vertex buffer
		IDirect3DSurface9* bb;
		videoOutputDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb);
		D3DSURFACE_DESC desc;
		bb->GetDesc(&desc);
		videoOutputDevice->CreateVertexBuffer(6*sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &vertexBuffer, NULL);
		// x, y, z, rhw, tu, tv
		CUSTOMVERTEX vertices[] =
		{
			{   0.0f,                0.0f,              0.0f, 1.0f, 0.0f - halfTexelWidth, 0.0f - halfTexelHeight, },
			{   (FLOAT)desc.Width,   0.0f,              0.0f, 1.0f, 1.0f - halfTexelWidth, 0.0f - halfTexelHeight, },
			{   0.0f,               (FLOAT)desc.Height, 0.0f, 1.0f, 0.0f - halfTexelWidth, 1.0f - halfTexelHeight, },
			{   (FLOAT)desc.Width,  (FLOAT)desc.Height, 0.0f, 1.0f, 1.0f - halfTexelWidth, 1.0f - halfTexelHeight, },
		};
		bb->Release();
		void* pVertices;
		vertexBuffer->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
		memcpy(pVertices, vertices, sizeof(vertices));
		vertexBuffer->Unlock();
	}

	// initialize the pixelBuffer
	VideoBus::init(width, height);
}

void VideoBusWinD3D::release()
{
	if (this->videoOutputDevice != NULL) {
		if (vertexBuffer != NULL) {
			vertexBuffer->Release();
			vertexBuffer = NULL;
		}
		if (combinedTexture != NULL) {
			combinedTexture->Release();
			combinedTexture = NULL;
		}
	}

	VideoBus::release();
}

void VideoBusWinD3D::render()
{
	VideoBus::render();

	combinedTexture->LockRect(0, &combinedBufferLock, NULL, D3DLOCK_DISCARD |  D3DLOCK_NOSYSLOCK);
	memcpy(combinedBufferLock.pBits, pixelBuffer, pixelBufferSize);
	combinedTexture->UnlockRect(0);

	videoOutputDevice->SetTexture(0, combinedTexture);
	videoOutputDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	videoOutputDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	videoOutputDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
	videoOutputDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	videoOutputDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	videoOutputDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	videoOutputDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	videoOutputDevice->SetStreamSource(0, vertexBuffer, 0, sizeof(CUSTOMVERTEX));
	videoOutputDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}
