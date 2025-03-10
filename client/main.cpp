#define WM_STOP_PLAYBACK (WM_USER + 0)
#include "client.h"
#include "windowHandler.h"
#include "audioHandler.h"
#include "..\helper.h"
#include <iostream>
#include <stdexcept>
#include <thread>

// window handle
HWND hwnd = NULL;
// socket instance
SOCKET conn = INVALID_SOCKET;
// audio handler instance
AudioHandler audioHandler;
// chunk variables
const char* curChunkDataArrPtr = nullptr;
const char* nextChunkDataArrPtr = nullptr;
int curChunkSize, nextChunkSize;
// manual stop trigger
bool manualStop = false;

void ReleaseNextChunk()
{
	delete[] nextChunkDataArrPtr;
	nextChunkDataArrPtr = nullptr;
	nextChunkSize = -1;
}

void CheckStopAudio()
{
	if (audioHandler.IsPlaying())
	{
		manualStop = true;
		audioHandler.StopAudio();
	}
}

void HandleAudioPlayback()
{
	try
	{
		while (true)
		{
			// request next chunk
			helper::SendMsg(conn, helper::SEND_CHUNK_MSG);
			// play chunk
			audioHandler.PlayChunk(curChunkDataArrPtr, curChunkSize);
			// last chunk or manual stop triggered
			if (nextChunkDataArrPtr == nullptr || manualStop)
			{
				break;
			}
			// swap to next chunk
			memcpy((void*)curChunkDataArrPtr, nextChunkDataArrPtr, nextChunkSize);
			curChunkSize = nextChunkSize;
			ReleaseNextChunk();
		}
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, e.what(), "AudioPlayback Failed Debug", MB_OK);
	}
	try
	{
		// release next chunk
		if (nextChunkDataArrPtr != nullptr)
		{
			ReleaseNextChunk();
		}
		// release cur chunk
		delete[] curChunkDataArrPtr;
		curChunkDataArrPtr = nullptr;
		curChunkSize = -1;
		// signal close socket
		helper::SendMsg(conn, helper::CLOSE_SOCKET_MSG);
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, e.what(), "AudioPlayback CloseSocket Failed Debug", MB_OK);
	}
}

void HandleSocket()
{
	// socket handler instance
	ClientSocket clientSocket = ClientSocket("192.168.2.105", 1234);
	// audio playback thread
	std::thread audioPlaybackThread;
	try 
	{
		conn = clientSocket.CreateAndConnect();
		bool isSocketOpen = true;
		std::cout << "Connection established" << std::endl;
		// send start streaming msg
		helper::SendMsg(conn, helper::START_STREAMING_MSG);
		// recv buffer
		char recvBuffer[helper::RECV_BUFFER_LEN];
		while (isSocketOpen)
		{
			int recvByteCount = recv(conn, recvBuffer, helper::RECV_BUFFER_LEN, 0);
			if (recvByteCount <= 0)
			{
				if (recvByteCount < 0)
				{
					MessageBoxA(NULL, "Failed to receive msg.", "ClientSocket Error", MB_OK);
				}
				isSocketOpen = false;
				continue;
			}
			// get msg from recv buffer
			std::string recvMsgStr = "";
			for (int i = 0; i < recvByteCount; i++)
			{
				char ch = recvBuffer[i];
				recvMsgStr += ch;
			}
			const char* recvMsg = recvMsgStr.c_str();
			// close socket msg
			if (strcmp(recvMsg, helper::CLOSE_SOCKET_MSG) == 0)
			{
				isSocketOpen = false;
			}
			else
			{
				if (curChunkDataArrPtr == nullptr)
				{
					curChunkDataArrPtr = new char[recvByteCount];
					memcpy((void*)curChunkDataArrPtr, recvMsg, recvByteCount);
					curChunkSize = recvByteCount;
					// set audio output device 
					audioHandler.InitAudioDevice(hwnd);
					// create secondary buffer and set notify event
					audioHandler.CreateSecondaryBuffer(curChunkSize);
					// create audio playback thread
					audioPlaybackThread = std::thread(HandleAudioPlayback);
				}
				else if (nextChunkDataArrPtr == nullptr)
				{
					nextChunkDataArrPtr = new char[recvByteCount];
					memcpy((void*)nextChunkDataArrPtr, recvMsg, recvByteCount);
					nextChunkSize = recvByteCount;
				}
			}
		}
	} 
	catch (std::runtime_error e) 
	{
		MessageBoxA(NULL, e.what(), "SocketHandle Failed Debug", MB_OK);
	}
	try
	{
		// close window
		if (IsWindowVisible(hwnd))
		{
			PostMessageA(hwnd, WM_DESTROY, 0, 0);
		}
		if (audioPlaybackThread.joinable())
		{
			audioPlaybackThread.join();
		}
		// release audio
		audioHandler.Release();
		std::cout << "Audio memory released" << std::endl;
		// release socket
		clientSocket.Close();
		conn = INVALID_SOCKET;
		std::cout << "Client socket memory released" << std::endl;
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, e.what(), "SocketHandle Cleanup Failed Debug", MB_OK);
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// window handler instance
	WindowHandler windowHandler;
	// socket handle thread
	std::thread socketHandleThread;
	try 
	{
		// create and show window
		hwnd = windowHandler.CreateShowWindow(hInstance);
		// socket handler thread
		socketHandleThread = std::thread(HandleSocket);
		// window message loop
		MSG msg;
		while (true)
		{
			int msgRes = GetMessageA(&msg, NULL, 0, 0);
			// error
			if (msgRes == -1)
			{
				continue;
			}
			// manual stop triggered 
			if (msg.message == WM_STOP_PLAYBACK)
			{
				CheckStopAudio();
			}
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			// close window
			if (msgRes == 0)
			{
				CheckStopAudio();
				break;
			}
		}
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, e.what(), "Window Exception", MB_OK);
	}
	try
	{
		if (socketHandleThread.joinable())
		{
			socketHandleThread.join();
		}
		// release window
		windowHandler.Release();
		hwnd = NULL;
		std::cout << "Window memory released" << std::endl;
	}
	catch (std::runtime_error e)
	{
		MessageBoxA(NULL, e.what(), "Window Cleanup Exception", MB_OK);
	}
	return 0;
}
