#include "client.h"
#include "windowHandler.h"
#include "audioHandler.h"
#include "..\helper.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <mutex>

// io mutex
std::mutex ioMutex;
// socket open check
bool isSocketOpen;
// window handle
HWND hwnd = NULL;
// socket instance
SOCKET conn = INVALID_SOCKET;
// audio handler instance
AudioHandler audioHandler;
// audio playback thread
std::thread audioPlaybackThread;
// chunk variables
const char* curChunkDataArrPtr = nullptr;
const char* nextChunkDataArrPtr = nullptr;
int curChunkSize, nextChunkSize;
// manual stop trigger
bool manualStop = false;
// pause/unpause
bool pause = false;

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
		if (!pause)
		{
			manualStop = true;
		}
		audioHandler.StopAudio();
	}
}

void AudioPlaybackCleanup()
{
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
		// release audio
		audioHandler.Release();
		// lock io
		ioMutex.lock();
		std::cout << "Audio memory released" << std::endl;
		// unlock io
		ioMutex.unlock();
		// close window
		if (IsWindowVisible(hwnd))
		{
			PostMessageA(hwnd, WM_DESTROY, 0, 0);
		}
	}
	catch (std::runtime_error e)
	{
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
	}
}

void HandleAudioPlayback()
{
	try
	{
		while (true)
		{
			if (!audioHandler.AfterUnpause())
			{
				// request next chunk
				helper::SendMsg(conn, helper::SEND_CHUNK_MSG);
			}
			// play chunk
			audioHandler.PlayChunk(curChunkDataArrPtr, curChunkSize);
			// pause
			if (pause)
			{
				audioHandler.Pause();		
				return;
			}
			// last chunk or manual stop triggered
			else if (nextChunkDataArrPtr == nullptr || manualStop)
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
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
	}
	AudioPlaybackCleanup();
}

void HandleUserInput()
{
	try
	{
		std::string input;
		while (true)
		{
			// lock io
			ioMutex.lock();
			std::cin >> input;
			// unlock io
			ioMutex.unlock();
			// check after input
			if (!isSocketOpen)
			{
				break;
			}
			if (input == "close")
			{
				helper::SendMsg(conn, helper::CLOSE_SOCKET_MSG);
				if (!pause)
				{
					CheckStopAudio();
				}
				else 
				{
					AudioPlaybackCleanup();
				}
				break;
			}
			else if (input == "pause")
			{
				if (!pause && audioHandler.IsPlaying())
				{
					pause = true;
					CheckStopAudio();
				}
			}
			else if (input == "unpause")
			{
				if (pause && audioHandler.IsPlaying())
				{
					pause = false;
					audioHandler.Unpause();
					if (audioPlaybackThread.joinable())
					{
						audioPlaybackThread.join();
					}
					audioPlaybackThread = std::thread(HandleAudioPlayback);
				}
			}
			else if (input == "stop")
			{
				if (!pause)
				{
					CheckStopAudio();
				}
			}
		}
	}
	catch (std::runtime_error e)
	{
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
	}
}

void HandleSocket()
{
	// get socket ip and port
	const char* ipv4Addr;
	int port;
	helper::GetSocketConfig(ipv4Addr, port);
	// socket handler instance
	ClientSocket clientSocket = ClientSocket(ipv4Addr, port);
	// user input thread
	std::thread userInputThread;
	try 
	{
		conn = clientSocket.CreateAndConnect();
		isSocketOpen = true;
		// lock io
		ioMutex.lock();
		std::cout << "Connection established" << std::endl;
		// unlock io
		ioMutex.unlock();
		userInputThread = std::thread(HandleUserInput);
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
					// lock io
					ioMutex.lock();
					std::cout << "Failed to receive msg." << std::endl;
					// unlock io
					ioMutex.unlock();
				}
				isSocketOpen = false;
				continue;
			}
			// get msg from recv buffer
			char recvMsg[recvByteCount];
			helper::GetRecvMsg(recvBuffer, recvByteCount, recvMsg);
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
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
		isSocketOpen = false;
	}
	try
	{
		if (audioPlaybackThread.joinable())
		{
			audioPlaybackThread.join();
		}
		if (userInputThread.joinable())
		{
			userInputThread.join();
		}
		// release socket
		clientSocket.Close();
		delete[] ipv4Addr;
		conn = INVALID_SOCKET;
		// lock io
		ioMutex.lock();
		std::cout << "Client socket memory released" << std::endl;
		// unlock io
		ioMutex.unlock();
	}
	catch (std::runtime_error e)
	{
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
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
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
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
		// lock io
		ioMutex.lock();
		std::cout << "Window memory released" << std::endl;
		// unlock io
		ioMutex.unlock();
	}
	catch (std::runtime_error e)
	{
		// lock io
		ioMutex.lock();
		std::cout << e.what() << std::endl;
		// unlock io
		ioMutex.unlock();
	}
	return 0;
}
