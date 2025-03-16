#include "audioHandler.h"
#include <stdexcept>
#include <initguid.h>
DEFINE_GUID(IID_IDirectSoundNotify8, 0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);

AudioHandler::AudioHandler()
{
	this->pausedPos = 0;
	// initialize to check value on release
	this->audioDevicePtr = nullptr;
	this->secondaryBufferPtr = nullptr;
	this->lpNotify = nullptr;
	this->hNotifyOnPlayCursorStop = nullptr;
	// wave format
	ReadWavFormatData();
}

void AudioHandler::ReadWavFormatData()
{
	this->waveFormat = { };
	this->waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	this->waveFormat.nChannels = 2;
	this->waveFormat.nSamplesPerSec = 44100;
	this->waveFormat.wBitsPerSample = 16;
	this->waveFormat.nBlockAlign = this->waveFormat.nChannels * this->waveFormat.wBitsPerSample / 8;
	this->waveFormat.nAvgBytesPerSec = this->waveFormat.nSamplesPerSec * this->waveFormat.nBlockAlign;
	this->waveFormat.cbSize = 0;
}

void AudioHandler::InitAudioDevice(HWND hwnd)
{
	// create audio device
	HRESULT deviceCreateRes = DirectSoundCreate8(NULL, &this->audioDevicePtr, NULL);
	if (FAILED(deviceCreateRes)) 
	{
		throw std::runtime_error("Failed to create audio device");
	}
	// set access level to this device object  
	HRESULT setCooperateLevelRes = this->audioDevicePtr->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if (FAILED(setCooperateLevelRes))
	{
		throw std::runtime_error("Failed to set audio device cooperate level");
	}
}

void AudioHandler::CreateSecondaryBuffer(int chunkSize)
{
	// secondary buffer desc
	this->bufferDesc = { };
	this->bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	// secondary buffer
	this->bufferDesc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS;
	this->bufferDesc.dwBufferBytes = chunkSize;
	this->bufferDesc.lpwfxFormat = &this->waveFormat;
	// create the secondary buffer
	HRESULT secondaryBufferCreateOut = this->audioDevicePtr->CreateSoundBuffer(&this->bufferDesc, &this->secondaryBufferPtr, NULL);
	if (FAILED(secondaryBufferCreateOut)) 
	{
		throw std::runtime_error("Failed to create a secondary buffer");
	}
	SetBufferNotificationEvent();
}

void AudioHandler::SetBufferNotificationEvent()
{
	// get access to secondary buffer notification system through the notify pointer
	bool requestBufferNotificationOut = this->secondaryBufferPtr->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&this->lpNotify);
	if (FAILED(requestBufferNotificationOut))
	{
		throw std::runtime_error("Failed to request a buffer notification");
	}
	// set position to stop/end of play buffer
	this->positionNotify.dwOffset = DSBPN_OFFSETSTOP;
	// create notification event
	if (this->hNotifyOnPlayCursorStop != nullptr)
	{
		CloseHandle(this->hNotifyOnPlayCursorStop);
		hNotifyOnPlayCursorStop = nullptr;
	}
	this->hNotifyOnPlayCursorStop = CreateEventA(NULL, true, false, "NotifyOnPlayCursorStop");
	if (hNotifyOnPlayCursorStop == NULL)
	{
		throw std::runtime_error("Failed to create the notify event");
	}
	// set notification event
	this->positionNotify.hEventNotify = this->hNotifyOnPlayCursorStop;
	// set notification position event on secondary buffer
	HRESULT setNotificationPositionOut = this->lpNotify->SetNotificationPositions(1, &positionNotify);
	// release notify pointer
	this->lpNotify->Release();
	this->lpNotify = nullptr;
	if (FAILED(setNotificationPositionOut))
	{
		throw std::runtime_error("Failed to set notification position on buffer");
	}
}

void AudioHandler::CopyAudioToSecondaryBuffer(const char* chunkDataArrPtr)
{
	int lockOffset = 0;
	int lockSize = 0;
	void* dataPtr;
	DWORD dataSize;
	// lock the buffer
	HRESULT lockSecBufferRes = this->secondaryBufferPtr->Lock(lockOffset, lockSize, &dataPtr, &dataSize, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (FAILED(lockSecBufferRes))
	{
		throw std::runtime_error("Failed to lock the secondary buffer");
	}
	// copy the audio data to the buffer
	memcpy(dataPtr, chunkDataArrPtr, dataSize);
	// unlock the buffer		
	HRESULT unlockSecBufferRes = this->secondaryBufferPtr->Unlock(dataPtr, dataSize, NULL, 0);
	if (FAILED(unlockSecBufferRes))
	{
		throw std::runtime_error("Failed to unlock the secondary buffer");
	}
}
void AudioHandler::PlayChunk(const char* chunkDataArrPtr, int chunkSize)
{
	// last chunk create smaller buffer
	if (chunkSize < this->bufferDesc.dwBufferBytes)
	{
		// close notification event
		if (this->hNotifyOnPlayCursorStop != nullptr)
		{
			CloseHandle(this->hNotifyOnPlayCursorStop);
			this->hNotifyOnPlayCursorStop = nullptr;
		}
		// close secondary buffer
		if (this->secondaryBufferPtr != nullptr)
		{
			this->secondaryBufferPtr->Release();
			this->secondaryBufferPtr = nullptr;
		}
		// create secondary buffer and set notify event
		CreateSecondaryBuffer(chunkSize);
	}
	CopyAudioToSecondaryBuffer(chunkDataArrPtr);
	// play
	PlayAudio();
	DWORD waitRes = WaitForSingleObject(this->hNotifyOnPlayCursorStop, INFINITE);
	if (waitRes != WAIT_OBJECT_0)
	{
		throw std::runtime_error("Timeout occured on wait buffer stop event");
	}
	// reset notify event
	ResetEvent(this->hNotifyOnPlayCursorStop);
}

void AudioHandler::PlayAudio()
{
	HRESULT playRes = this->secondaryBufferPtr->Play(0, 0, 0);
	if (FAILED(playRes)) 
	{
		throw std::runtime_error("Failed to play the audio chunk");
	}
}

bool AudioHandler::IsPlaying()
{
	return this->secondaryBufferPtr != nullptr;
}

void AudioHandler::Pause()
{
	this->secondaryBufferPtr->GetCurrentPosition(&this->pausedPos, NULL);
}

void AudioHandler::Unpause()
{
	this->secondaryBufferPtr->SetCurrentPosition(this->pausedPos);
}

bool AudioHandler::AfterUnpause()
{
	if (this->pausedPos == 0)
	{
		return false;
	}
	this->pausedPos = 0;
	return true;
}

void AudioHandler::StopAudio()
{
	HRESULT stopRes = this->secondaryBufferPtr->Stop();
	if (FAILED(stopRes))
	{
		throw std::runtime_error("Failed to stop the audio chunk");
	}
}

void AudioHandler::Release()
{
	// notification event
	if (this->hNotifyOnPlayCursorStop != nullptr)
	{
		CloseHandle(this->hNotifyOnPlayCursorStop);
		this->hNotifyOnPlayCursorStop = nullptr;
	}
	// notification system
	if (this->lpNotify != nullptr)
	{
		this->lpNotify->Release();
		this->lpNotify = nullptr;
	}
	// secondary buffer
	if (this->secondaryBufferPtr != nullptr)
	{
		this->secondaryBufferPtr->Release();
		this->secondaryBufferPtr = nullptr;
	}
	// audio device
	if (this->audioDevicePtr != nullptr)
	{
		this->audioDevicePtr->Release();
		this->audioDevicePtr = nullptr;
	}
}
