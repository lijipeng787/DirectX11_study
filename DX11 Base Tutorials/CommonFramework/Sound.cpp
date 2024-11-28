#include "stdafx.h"

#include "Sound.h"

struct WaveHeaderType {

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

SoundClass::SoundClass() {}

SoundClass::~SoundClass() {}

bool SoundClass::Initialize(HWND hwnd, char *sound_filename) {

  bool result;

  result = InitializeDirectSound(hwnd);
  if (!result) {
    return false;
  }

  result =
      LoadWaveFile(sound_filename, &secondary_buffer_, &secondary_3D_buffer_);
  if (!result) {
    return false;
  }

  result = PlayWaveFile();
  if (!result) {
    return false;
  }

  return true;
}

void SoundClass::Shutdown() {

  ShutdownWaveFile(&secondary_buffer_, &secondary_3D_buffer_);

  ShutdownDirectSound();
}

bool SoundClass::InitializeDirectSound(HWND hwnd) {

  HRESULT result;
  DSBUFFERDESC bufferDesc;
  WAVEFORMATEX waveFormat;

  // Initialize the direct sound interface pointer for the default sound device.
  result = DirectSoundCreate8(NULL, &sound_device_, NULL);
  if (FAILED(result)) {
    return false;
  }

  // Set the cooperative level to priority so the format of the primary sound
  // buffer can be modified.
  result = sound_device_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
  if (FAILED(result)) {
    return false;
  }

  // Setup the primary buffer description.
  bufferDesc.dwSize = sizeof(DSBUFFERDESC);
  bufferDesc.dwFlags =
      DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
  bufferDesc.dwBufferBytes = 0;
  bufferDesc.dwReserved = 0;
  bufferDesc.lpwfxFormat = NULL;
  bufferDesc.guid3DAlgorithm = GUID_NULL;

  // Get control of the primary sound buffer on the default sound device.
  result =
      sound_device_->CreateSoundBuffer(&bufferDesc, &primary_buffer_, NULL);
  if (FAILED(result)) {
    return false;
  }

  // Setup the format of the primary sound bufffer.
  // In this case it is a .WAV file recorded at 44,100 samples per second in
  // 16-bit stereo (cd audio format).
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nSamplesPerSec = 44100;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nChannels = 2;
  waveFormat.nBlockAlign =
      (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
  waveFormat.nAvgBytesPerSec =
      waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  // Set the primary buffer to be the wave format specified.
  result = primary_buffer_->SetFormat(&waveFormat);
  if (FAILED(result)) {
    return false;
  }

  // Obtain a listener interface.
  result = primary_buffer_->QueryInterface(IID_IDirectSound3DListener8,
                                           (LPVOID *)&listener_);
  if (FAILED(result)) {
    return false;
  }

  // Set the initial position of the listener to be in the middle of the scene.
  listener_->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);

  return true;
}

void SoundClass::ShutdownDirectSound() {

  if (listener_) {
    listener_->Release();
    listener_ = 0;
  }

  // Release the primary sound buffer pointer.
  if (primary_buffer_) {
    primary_buffer_->Release();
    primary_buffer_ = 0;
  }

  // Release the direct sound interface pointer.
  if (sound_device_) {
    sound_device_->Release();
    sound_device_ = 0;
  }
}

bool SoundClass::LoadWaveFile(char *filename,
                              IDirectSoundBuffer8 **secondaryBuffer,
                              IDirectSound3DBuffer8 **secondary3DBuffer) {

  int error;
  FILE *filePtr;
  size_t count;
  WaveHeaderType waveFileHeader;
  WAVEFORMATEX waveFormat;
  DSBUFFERDESC bufferDesc;
  HRESULT result;
  IDirectSoundBuffer *tempBuffer;
  unsigned char *waveData;
  unsigned char *bufferPtr;
  unsigned long bufferSize;

  // Open the wave file in binary.
  error = fopen_s(&filePtr, filename, "rb");
  if (error != 0) {
    return false;
  }

  // Read in the wave file header.
  count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
  if (count != 1) {
    return false;
  }

  // Check that the chunk ID is the RIFF format.
  if ((waveFileHeader.chunkId[0] != 'R') ||
      (waveFileHeader.chunkId[1] != 'I') ||
      (waveFileHeader.chunkId[2] != 'F') ||
      (waveFileHeader.chunkId[3] != 'F')) {
    return false;
  }

  // Check that the file format is the WAVE format.
  if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
      (waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E')) {
    return false;
  }

  // Check that the sub chunk ID is the fmt format.
  if ((waveFileHeader.subChunkId[0] != 'f') ||
      (waveFileHeader.subChunkId[1] != 'm') ||
      (waveFileHeader.subChunkId[2] != 't') ||
      (waveFileHeader.subChunkId[3] != ' ')) {
    return false;
  }

  // Check that the audio format is WAVE_FORMAT_PCM.
  if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM) {
    return false;
  }

  // Check that the wave file was recorded in mono format.
  if (waveFileHeader.numChannels != 1) {
    return false;
  }

  // Check that the wave file was recorded at a sample rate of 44.1 KHz.
  if (waveFileHeader.sampleRate != 44100) {
    return false;
  }

  // Ensure that the wave file was recorded in 16 bit format.
  if (waveFileHeader.bitsPerSample != 16) {
    return false;
  }

  // Check for the data chunk header.
  if ((waveFileHeader.dataChunkId[0] != 'd') ||
      (waveFileHeader.dataChunkId[1] != 'a') ||
      (waveFileHeader.dataChunkId[2] != 't') ||
      (waveFileHeader.dataChunkId[3] != 'a')) {
    return false;
  }

  // Set the wave format of secondary buffer that this wave file will be loaded
  // onto.
  waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  waveFormat.nSamplesPerSec = 44100;
  waveFormat.wBitsPerSample = 16;
  waveFormat.nChannels = 1;
  waveFormat.nBlockAlign =
      (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
  waveFormat.nAvgBytesPerSec =
      waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  // Set the buffer description of the secondary sound buffer that the wave file
  // will be loaded onto.
  bufferDesc.dwSize = sizeof(DSBUFFERDESC);
  bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
  bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
  bufferDesc.dwReserved = 0;
  bufferDesc.lpwfxFormat = &waveFormat;
  bufferDesc.guid3DAlgorithm = GUID_NULL;

  // Create a temporary sound buffer with the specific buffer settings.
  result = sound_device_->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
  if (FAILED(result)) {
    return false;
  }

  // Test the buffer format against the direct sound 8 interface and create the
  // secondary buffer.
  result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8,
                                      (void **)&*secondaryBuffer);
  if (FAILED(result)) {
    return false;
  }

  // Release the temporary buffer.
  tempBuffer->Release();
  tempBuffer = 0;

  // Move to the beginning of the wave data which starts at the end of the data
  // chunk header.
  fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

  // Create a temporary buffer to hold the wave file data.
  waveData = new unsigned char[waveFileHeader.dataSize];
  if (!waveData) {
    return false;
  }

  // Read in the wave file data into the newly created buffer.
  count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
  if (count != waveFileHeader.dataSize) {
    return false;
  }

  // Close the file once done reading.
  error = fclose(filePtr);
  if (error != 0) {
    return false;
  }

  // Lock the secondary buffer to write wave data into it.
  result = (*secondaryBuffer)
               ->Lock(0, waveFileHeader.dataSize, (void **)&bufferPtr,
                      (DWORD *)&bufferSize, NULL, 0, 0);
  if (FAILED(result)) {
    return false;
  }

  // Copy the wave data into the buffer.
  memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

  // Unlock the secondary buffer after the data has been written to it.
  result = (*secondaryBuffer)->Unlock((void *)bufferPtr, bufferSize, NULL, 0);
  if (FAILED(result)) {
    return false;
  }

  // Release the wave data since it was copied into the secondary buffer.
  delete[] waveData;
  waveData = 0;

  // Get the 3D interface to the secondary sound buffer.
  result = (*secondaryBuffer)
               ->QueryInterface(IID_IDirectSound3DBuffer8,
                                (void **)&*secondary3DBuffer);
  if (FAILED(result)) {
    return false;
  }

  return true;
}

void SoundClass::ShutdownWaveFile(IDirectSoundBuffer8 **secondaryBuffer,
                                  IDirectSound3DBuffer8 **secondary3DBuffer) {

  if (*secondary3DBuffer) {
    (*secondary3DBuffer)->Release();
    *secondary3DBuffer = 0;
  }

  if (*secondaryBuffer) {
    (*secondaryBuffer)->Release();
    *secondaryBuffer = 0;
  }
}

bool SoundClass::PlayWaveFile() {

  HRESULT result;
  float positionX, positionY, positionZ;

  // Set the 3D position of where the sound should be located.
  positionX = -2.0f;
  positionY = 0.0f;
  positionZ = 0.0f;

  // Set position at the beginning of the sound buffer.
  result = secondary_buffer_->SetCurrentPosition(0);
  if (FAILED(result)) {
    return false;
  }

  // Set volume of the buffer to 100%.
  result = secondary_buffer_->SetVolume(DSBVOLUME_MAX);
  if (FAILED(result)) {
    return false;
  }

  // Set the 3D position of the sound.
  secondary_3D_buffer_->SetPosition(positionX, positionY, positionZ,
                                    DS3D_IMMEDIATE);

  // Play the contents of the secondary sound buffer.
  result = secondary_buffer_->Play(0, 0, 0);
  if (FAILED(result)) {
    return false;
  }

  return true;
}
