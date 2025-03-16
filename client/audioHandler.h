#pragma once
#include <dsound.h>

class AudioHandler
{
	public:	
		AudioHandler();
		void InitAudioDevice(HWND hwnd);
		void CreateSecondaryBuffer(int chunkSize);
		void PlayChunk(const char* chunkDataArrPtr, int chunkSize);
		bool IsPlaying();
		void Pause();
		void Unpause();
		bool AfterUnpause(); 
		void StopAudio();
		void Release();

	private:
		DWORD pausedPos;
		WAVEFORMATEX waveFormat;
		LPDIRECTSOUNDNOTIFY8 lpNotify;
		DSBPOSITIONNOTIFY positionNotify;
		HANDLE hNotifyOnPlayCursorStop;
		DSBUFFERDESC bufferDesc;
		LPDIRECTSOUNDBUFFER secondaryBufferPtr;
		LPDIRECTSOUND8 audioDevicePtr;
		void ReadWavFormatData();
		void SetBufferNotificationEvent();
		void CopyAudioToSecondaryBuffer(const char* chunkDataArrPtr);
		void PlayAudio();
};
