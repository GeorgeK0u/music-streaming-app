#include "server.h"
#include "fileHandler.h"
#include "..\helper.h"
#include <iostream>
#include <stdexcept>

SOCKET conn = INVALID_SOCKET;
FileHandler fileHandler(helper::RECV_BUFFER_LEN);

void SendChunk()
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

int main()
{
	int maxConnNum = 1;
	ServerSocket serverSocket = ServerSocket("192.168.2.105", 1234, maxConnNum);
	try 
	{
		conn = serverSocket.CreateAndAccept();
		std::cout << "Client accepted" << std::endl;
		bool isSocketOpen = true;
		// conn loop
		char recvBuffer[helper::RECV_BUFFER_LEN];
		while (isSocketOpen)
		{
			int recvByteCount = recv(conn, recvBuffer, helper::RECV_BUFFER_LEN, 0);
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
				helper::SendMsg(conn, helper::CLOSE_SOCKET_MSG);
				isSocketOpen = false;
			}
			else if (strcmp(recvMsg, helper::START_STREAMING_MSG) == 0)
			{
				std::cout << "Start streaming msg received from client" << std::endl;
				// path is relative to .output dir
				/* const char* fileName = "songs/nft-song.wav"; */
				const char* fileName = "songs/track1.wav";
				fileHandler.CheckReadFile(fileName);
				// send chunk
				SendChunk();
			}
			else if (strcmp(recvMsg, helper::SEND_CHUNK_MSG) == 0)
			{
				std::cout << "Client requested next chunk" << std::endl;
				// send chunk
				SendChunk();
			}
			else
			{
				std::cout << "CLIENT: " << recvMsg << std::endl;
			}
		}
	} 
	catch (std::runtime_error e) 
	{
		std::cout << "An exception occured: " << e.what() << std::endl;
	}
	// cleanup
	try
	{
		// release file memory
		fileHandler.Release();
		std::cout << "File memory released" << std::endl; 
		// release socket memory
		serverSocket.Close();
		conn = INVALID_SOCKET;
		std::cout << "Server socket memory released" << std::endl; 
	}
	catch (std::runtime_error e)
	{
		std::cout << "A cleanup exception occured: " << e.what() << std::endl;
	}
	return 0;
}
