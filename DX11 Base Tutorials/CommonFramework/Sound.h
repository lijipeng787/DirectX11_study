#pragma once

#include <dsound.h>

class SoundClass{
public:
	SoundClass();

	SoundClass(const SoundClass& rhs) = delete;

	SoundClass& operator=(const SoundClass& rhs) = delete;
	
	~SoundClass();
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
	IDirectSound8* sound_device_ = nullptr;

	IDirectSoundBuffer* primary_buffer_ = nullptr;

	IDirectSound3DListener8* listener_ = nullptr;

	IDirectSoundBuffer8* secondary_buffer_ = nullptr;

	IDirectSound3DBuffer8* secondary_3D_buffer_ = nullptr;
};
