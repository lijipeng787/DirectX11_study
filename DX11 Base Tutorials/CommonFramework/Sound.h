#ifndef _SOUNDCLASS_H_
#define _SOUNDCLASS_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>

class SoundClass{
public:
	SoundClass();

	SoundClass(const SoundClass& rhs) = delete;

	SoundClass& operator=(const SoundClass& rhs) = delete;
	
	~SoundClass();
private:
	struct WaveHeaderType{

		char chunkId[4];
		unsigned long chunkSize;
		char format[4];
		char subChunkId[4];
		unsigned long subChunkSize;
		unsigned short audioFormat;
		unsigned short numChannels;
		unsigned long sampleRate;
		unsigned long bytesPerSecond;
		unsigned short blockAlign;
		unsigned short bitsPerSample;
		char dataChunkId[4];
		unsigned long dataSize;
	};
public:
	bool Initialize(HWND, char *sound_filename);

	void Shutdown();
private:
	bool InitializeDirectSound(HWND);
	
	void ShutdownDirectSound();

	bool LoadWaveFile(char*, IDirectSoundBuffer8**, IDirectSound3DBuffer8**);

	void ShutdownWaveFile(IDirectSoundBuffer8**, IDirectSound3DBuffer8**);

	bool PlayWaveFile();
private:
	IDirectSound8* m_DirectSound = nullptr;

	IDirectSoundBuffer* m_primaryBuffer = nullptr;

	IDirectSound3DListener8* m_listener = nullptr;

	IDirectSoundBuffer8* m_secondaryBuffer1 = nullptr;

	IDirectSound3DBuffer8* m_secondary3DBuffer1 = nullptr;
};

#endif