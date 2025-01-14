#include <WinSock2.h>
#include <iostream>
#include "server.h"
#include "../helper.h"
#include <stdexcept>

SOCKET conn;
bool isSocketOpen;

int main()
{
	int maxConnNum = 1;
	ServerSocket listenSocket = ServerSocket("192.168.2.105", 1234, maxConnNum);
	try 
	{
		InitWinsockDll();
		conn = listenSocket.InitAndAccept();
		isSocketOpen = true;
		// conn loop
		char recvBuffer[helper::RECV_BUFFER_LEN];
		while (isSocketOpen)
		{
			int recvByteCount = recv(conn, recvBuffer, helper::RECV_BUFFER_LEN, 0);
			if (recvByteCount < 0)
			{
				std::cout << "Failed to receive msg. Error code: " << WSAGetLastError() << std::endl;
				isSocketOpen = false;
				continue;
			}
			std::cout << "Received: " << recvByteCount << " bytes from a " << helper::RECV_BUFFER_LEN << " bytes buffer" << std::endl;
			// get msg from recv buffer
			int bufferCharCount = sizeof(recvBuffer) / sizeof(recvBuffer[0]);
			std::string recvMsg = GetRecvBufferMsg(recvBuffer, bufferCharCount, recvByteCount);
			std::cout << "Received msg: " << recvMsg << std::endl;
			// close socket msg
			if (recvMsg == helper::CLOSE_SOCKET_MSG)
			{
				bool ok = SendMsg(conn, helper::CLOSE_SOCKET_MSG, true);
				if (ok)
				{
					isSocketOpen = false;
				}
				continue;
			}
			std::cout << "CLIENT: " << recvMsg << std::endl;
		}
		std::cout << "Recv loop closed" << std::endl;
		CloseSocket(conn);
		std::cout << "Server socket closed" << std::endl;
		CloseWinsockDll();
		std::cout << "Winsock dll closed" << std::endl;
	} 
	catch (std::runtime_error e) 
	{
		std::cout << "An exception occured: " << e.what() << std::endl;
	}
	return 0;
}
