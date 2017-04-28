

#ifndef _SOUND_H_
#define _SOUND_H_

#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"winmm.lib")

#include <Windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <stdio.h>

class Sound
{
private:
	// the struct is for the .wav file format
	// for information, check this: http://soundfile.sapp.org/doc/WaveFormat/
	struct WaveHeaderType 
	{
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
	Sound();
	Sound(const Sound&);
	~Sound();
	
	bool Initialize(HWND);  // initialize DirectSound and load in the wav audio file
							// and then play it once
	void Shutdown();  // release the wav file and shutdown DirectSound

private:
	bool InitializeDirectSound(HWND);
	void ShutdownDirectSound();

	bool LoadWaveFile(char*, IDirectSoundBuffer8**);
	void ShutdownWaveFile(IDirectSoundBuffer8**);

	bool PlayWaveFile();

private:
	IDirectSound8* m_DirectSound;
	IDirectSoundBuffer* m_primaryBuffer;
	IDirectSoundBuffer8* m_secondaryBuffer;  // only load in one sound 
											 // need to modify if there are more sound

};

#endif