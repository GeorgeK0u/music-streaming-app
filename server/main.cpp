#include "server.h"
#include "fileHandler.h"
#include "..\helper.h"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <csignal>

// ctrl+c signal
bool termSignal;
// listening socket
ListenSocket listenSocket;

void SignalHandle(int sigum)
{
	termSignal = true;
	// close listening socket
	listenSocket.Close();
}

void SendChunk(SOCKET conn, FileHandler& fileHandler)
{
	char* chunkDataArrPtr = fileHandler.ReadChunk();
	// streaming finished
	if (chunkDataArrPtr == nullptr)
	{
		return;
	}
	// send chunk
	helper::SendChunkMsg(conn, chunkDataArrPtr, fileHandler.GetChunkSize());
	std::cout << "Sent chunk" << std::endl;
	delete[] chunkDataArrPtr;
}

void HandleClient(SOCKET conn)
{
	FileHandler fileHandler(helper::RECV_BUFFER_LEN);
	try
	{
		std::cout << "Client accepted" << std::endl;
		bool isSocketOpen = true;
		// conn loop
		char recvBuffer[helper::RECV_BUFFER_LEN];
		while (isSocketOpen)
		{
			const int recvByteCount = recv(conn, recvBuffer, helper::RECV_BUFFER_LEN, 0);
			if (recvByteCount <= 0)
			{
				if (recvByteCount < 0)
				{
					std::cout << "Failed to receive msg. Error code: " << WSAGetLastError() << std::endl;
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
				helper::SendMsg(conn, helper::CLOSE_SOCKET_MSG);
				isSocketOpen = false;
			}
			// start streaming msg
			else if (strcmp(recvMsg, helper::START_STREAMING_MSG) == 0)
			{
				std::cout << "Start streaming msg received from client" << std::endl;
				// path is relative to .output dir
				const char* fileName = "songs/nft-song.wav";
				/* const char* fileName = "songs/track1.wav"; */
				fileHandler.CheckReadFile(fileName);
				// send chunk
				SendChunk(conn, fileHandler);
			}
			// send chunk msg
			else if (strcmp(recvMsg, helper::SEND_CHUNK_MSG) == 0)
			{
				std::cout << "Client requested next chunk" << std::endl;
				// send chunk
				SendChunk(conn, fileHandler);
			}
			// debug msg
			else
			{
				std::cout << "CLIENT: " << recvMsg << std::endl;
			}
		}
	}
	catch (std::runtime_error e) 
	{
		std::cout << "Server socket an exception occured: " << e.what() << std::endl;
	}
	// cleanup
	try
	{
		// release file memory
		fileHandler.Release();
		std::cout << "File memory released" << std::endl; 
		// release socket memory
		helper::CloseSocket(conn);
		std::cout << "Server socket memory released" << std::endl; 
	}
	catch (std::runtime_error e)
	{
		std::cout << "Server socket a cleanup exception occured: " << e.what() << std::endl;
	}
}

int main()
{
	// ctrl+c handling
	signal(SIGINT, SignalHandle);
	// client threads
	const int maxConnNum = 2;
	std::thread clientThreads[maxConnNum];
	// client count
	int clientCount = 0;
	// ip
	const char* ipv4Addr = nullptr;
	try 
	{
		// get socket ip and port
		int port;
		helper::GetSocketConfig(ipv4Addr, port);
		// set socket to listening mode
		listenSocket.CreateAndListen(ipv4Addr, port, maxConnNum);
		// accept loop
		while (true)
		{
			// accept
			SOCKET conn = listenSocket.Accept();
			// client accepted
			clientCount++;
			clientThreads[clientCount-1] = std::thread(HandleClient, conn);
		}
	}
	catch (std::runtime_error e)
	{
		if (termSignal)
		{
			std::cout << "Ctrl+C got pressed. Termination signal triggered" << std::endl; 
		}
		else
		{
			std::cout << "Listen socket error: " << e.what() << std::endl;
		}
	}
	try
	{
		// close client conn threads
		for (int i = clientCount-1; i >= 0; i--)
		{
			if (!clientThreads[i].joinable())
			{
				continue;
			}
			clientThreads[i].join();
			clientCount--;
		}
		// close listening socket
		listenSocket.Close();
		// free listening socket ip dynamic memory
		if (ipv4Addr != nullptr)
		{
			delete[] ipv4Addr;
		}
		// close winsock dll
		helper::CloseWinsockDll();
		std::cout << "Socket dll memory released" << std::endl;
	}
	catch (std::runtime_error e)
	{
		std::cout << "Listen socket cleanup error: " << e.what() << std::endl;
	}
	return 0;
}
