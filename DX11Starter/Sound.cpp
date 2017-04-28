#include "Sound.h"
#include <iostream>
using namespace std;

Sound::Sound()
{
	m_DirectSound = 0;
	m_primaryBuffer = 0;
	m_secondaryBuffer = 0;

}

Sound::Sound(const Sound& other)
{

}

Sound::~Sound()
{
}

bool Sound::Initialize(HWND hwnd)
{
	bool result;
	
	// initialize direct sound and the primary sound buffer
	result = InitializeDirectSound(hwnd);
	if (!result)
	{
		return false;
	}

	// load a wave audio file onto a secondary buffer
	result = LoadWaveFile("../Assets/Sounds/applause3.wav", &m_secondaryBuffer);
	//for test
	if (result)
	{
		cout << result << endl;
	}
	if (!result)
	{
		return false;
	}

	// play the wave now that it has been loaded
	result = PlayWaveFile();
	if (!result)
	{
		return false;
	}

	return true;
}

void Sound::Shutdown()
{
	// release the secondary buffer
	ShutdownWaveFile(&m_secondaryBuffer);

	// shut down the direct sound API
	ShutdownDirectSound();

	return;
}

bool Sound::InitializeDirectSound(HWND hwnd)
{
	HRESULT result;
	DSBUFFERDESC bufferDesc;
	WAVEFORMATEX waveFormat;

	// Initialize the direct sound interface pointer for the default sound device
	// can be modified if we want to use a specific device 
	// (query the system for all the sound devices and then grab the pointer to 
	// the primary sound buffer for a specific device

	result = DirectSoundCreate8(NULL, &m_DirectSound, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// set the cooperative level to the priority so the format of the primary sound 
	// buffer can be modified
	result = m_DirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if (FAILED(result))
	{
		return false;
	}

	// setup the primary buffer description
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// get control of the primary sound buffer on the default sound device
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// setup the format of the primary sound buffer
	// In this case it is a wav file recorded at 22050Hz  176 kb/s (1 chnl) 
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = 22050;
	waveFormat.nAvgBytesPerSec = 176400;
	waveFormat.wBitsPerSample = waveFormat.nAvgBytesPerSec / waveFormat.nSamplesPerSec;
	waveFormat.nBlockAlign = ((waveFormat.wBitsPerSample) / 8) *
		waveFormat.nChannels;
	waveFormat.cbSize = 0;  
	// for information, check this: https://msdn.microsoft.com/en-us/library/windows/desktop/dd390970(v=vs.85).aspx

	// set the primary buffer to be the wave format specified
	result = m_primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(result))
	{
		return false;
	}

	return true;
	
}

// ShutdownDirectSound function handles releasing the primary buffer and DirectSound interfaces
void Sound::ShutdownDirectSound()
{
	//release the primary sound buffer pointer
	if (m_primaryBuffer)
	{
		m_primaryBuffer->Release();
		m_primaryBuffer = 0;
	}

	//release the direct sound interface pointer
	if (m_DirectSound)
	{
		m_DirectSound->Release();
		m_DirectSound = 0;
	}
	return;
}

// LoadWaveFile function is what handles loading in a .wav audio file and then copied the data onto 
// a new secondary buffer; changes needed from the tutorial if different format 
bool Sound::LoadWaveFile(char* filename, IDirectSoundBuffer8** secondaryBuffer)
{
	int error;
	FILE* filePtr;
	unsigned int count;
	WaveHeaderType waveFileHeader;
	WAVEFORMATEX waveFormat;
	DSBUFFERDESC bufferDesc;
	HRESULT result;
	IDirectSoundBuffer* tempBuffer;
	unsigned char* waveData;
	unsigned char* bufferPtr;
	unsigned long bufferSize;

	//open the wave file in binary
	error = fopen_s(
		&filePtr,		// a pointer to the file pointer that will receive the pointer to the opened file
		filename,		// file name
		"rb"			// type of access permitted; 
	);
	// rb mode means you won't need to port your code, and expect  to read a lot of the 
	// file and/or don't care about network performance, memory mapped Win32 files might also be an option 

	if (error != 0)
	{
		return false;
	}

	// read in the wave file header
	count = fread(&waveFileHeader, sizeof(waveFileHeader), 1, filePtr);
	if(count!=1)
	{
		return false;
	}

	//check that the chunk ID is the RIFF format
	if((waveFileHeader.chunkId[0]!='R') || (waveFileHeader.chunkId[1] != 'I') ||
		(waveFileHeader.chunkId[2] != 'F') || (waveFileHeader.chunkId[3] != 'F'))
	{
		return false;
	}
	// check that the file format is the WAVE format
	if ((waveFileHeader.format[0] != 'W') || (waveFileHeader.format[1] != 'A') ||
		(waveFileHeader.format[2] != 'V') || (waveFileHeader.format[3] != 'E'))
	{
		return false;
	}
	//check that the sub chunk ID is the fmt format
	if ((waveFileHeader.subChunkId[0] != 'f') || (waveFileHeader.subChunkId[1] != 'm') ||
		(waveFileHeader.subChunkId[1] != 't') || (waveFileHeader.subChunkId[3] != ' '))
	{
		return false;
	}
	//check that the audio format is WAVE_FORMAT_PCM 
	if (waveFileHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		return false;
	}
	//check that the wave file was recorded in mono format
	if (waveFileHeader.numChannels != 1)
	{
		return false;
	}
	//check that the wave file was recorded at a sample rate of 22050 HZ ;22050Hz  176 kb/s (1 chnl)
	if (waveFileHeader.sampleRate != 22050)
	{
		return false;
	}
	//ensure that the wave file was recorded in 8 bit format
	if (waveFileHeader.bytesPerSecond != 176400)
	{
		return false;
	}
	//check for the data chunk header
	if ((waveFileHeader.dataChunkId[0] != 'd') || (waveFileHeader.dataChunkId[1] != 'a') ||
		(waveFileHeader.dataChunkId[2] != 't') || (waveFileHeader.dataChunkId[3] != 'a'))
	{
		return false;
	}
	// Set the wave format of secondary buffer that this wave file will be loaded onto.
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 22050;
	waveFormat.nAvgBytesPerSec = 176400;
	waveFormat.nChannels = 1;
	waveFormat.wBitsPerSample = waveFormat.nAvgBytesPerSec/ waveFormat.nSamplesPerSec;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.cbSize = 0;
	//set the buffer description of the secondary sound buffer that the wave filw will be loaded onto
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = waveFileHeader.dataSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	//create a temporary sound buffer with the specific buffer settings
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if (FAILED(result))
	{
		return false;
	}

	//test the buffer format against the direct sound 8 interface and create the secondary buffer
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*secondaryBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//release the temporary buffer
	tempBuffer->Release();
	tempBuffer = 0;

	// load in the wave data from the audio file

	// Move to the beginning of the wave data which starts at the end of the data chunk header.
	fseek(filePtr, sizeof(WaveHeaderType), SEEK_SET);

	// Create a temporary buffer to hold the wave file data.
	waveData = new unsigned char[waveFileHeader.dataSize];
	if (!waveData)
	{
		return false;
	}

	// Read in the wave file data into the newly created buffer.
	count = fread(waveData, 1, waveFileHeader.dataSize, filePtr);
	if (count != waveFileHeader.dataSize)
	{
		return false;
	}

	// Close the file once done reading.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Lock the secondary buffer to write wave data into it.
	result = (*secondaryBuffer)->Lock(0, waveFileHeader.dataSize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	if (FAILED(result))
	{
		return false;
	}

	// Copy the wave data into the buffer.
	memcpy(bufferPtr, waveData, waveFileHeader.dataSize);

	// Unlock the secondary buffer after the data has been written to it.
	result = (*secondaryBuffer)->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
	if (FAILED(result))
	{
		return false;
	}

	// Release the wave data since it was copied into the secondary buffer.
	delete[] waveData;
	waveData = 0;

	return true;

}

// ShutdownWaveFile realse the secondary buffer
void Sound::ShutdownWaveFile(IDirectSoundBuffer8** secondaryBuffer)
{
	if (*secondaryBuffer)
	{
		(*secondaryBuffer)->Release();
		*secondaryBuffer = 0;
	}
	return;
}

// PlayWaveFile function will play the audio file stored in the secondary buffer
bool Sound::PlayWaveFile()
{
	HRESULT result;

	//set position at the beginning of the sound buffer
	result = m_secondaryBuffer->SetCurrentPosition(0);
	if (FAILED(result))
	{
		return false;
	}

	//set volume of the buffer to 100%
	result = m_secondaryBuffer->SetVolume(DSBVOLUME_MAX);
	if (FAILED(result))
	{
		return false;
	}

	//play the contents of the secondary sound buffer
	result = m_secondaryBuffer->Play(0, 0, 0);
	if (FAILED(result))
	{
		return false;
	}
	return true;
}