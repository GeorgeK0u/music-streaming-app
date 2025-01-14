#include <WinSock2.h>
#include <iostream>
#include "client.h"
#include "../helper.h"
#include <string>
#include <stdexcept>
// threading
#include <thread>
#include <mutex>

SOCKET conn;
bool isSocketOpen;
std::mutex ioMutex;

void HandleUserInputLoop()
{
	std::string input;
	// user input loop
	while (isSocketOpen)
	{
		{
			// lock writing output to console
			std::lock_guard<std::mutex> lock(ioMutex);
			std::cout << "Input: ";
		}
		// get input
		std::getline(std::cin, input);
		// close socket
		if (input == helper::CLOSE_SOCKET_KEY)
		{
			bool ok = SendMsg(conn, helper::CLOSE_SOCKET_MSG);
			if (!ok)
			{
				// lock writing output to console
				std::lock_guard<std::mutex> lock(ioMutex);
				std::cout << "Failed to send close socket msg" << std::endl;
				continue;
			}
			// lock writing output to console
			std::lock_guard<std::mutex> lock(ioMutex);
			std::cout << "Sent close socket msg" << std::endl;
			break;
		}
	}
	// lock writing output to console
	std::lock_guard<std::mutex> lock(ioMutex);
	std::cout << "User input loop closed" << std::endl;
}

int main()
{
	ClientSocket clientSocket = ClientSocket("192.168.2.105", 1234);
	try 
	{
		InitWinsockDll();
		conn = clientSocket.CreateAndConnect();
		isSocketOpen = true;
		{
			// lock writing output to console
			std::lock_guard<std::mutex> lock(ioMutex);
			std::cout << "Connection established!" << std::endl;
		}
		// create 2nd thread to handle recv loop
		std::thread userInputLoopThread(HandleUserInputLoop);
		// recv buffer
		char recvBuffer[helper::RECV_BUFFER_LEN];
		while (isSocketOpen)
		{
			int recvByteCount = recv(conn, recvBuffer, helper::RECV_BUFFER_LEN, 0);
			if (recvByteCount < 0)
			{
				// lock writing output to console
				std::lock_guard<std::mutex> lock(ioMutex);
				std::cout << "Failed to receive msg. Error code: " << WSAGetLastError() << std::endl;
				isSocketOpen = false;
				continue;
			}
			{
				// lock writing output to console
				std::lock_guard<std::mutex> lock(ioMutex);
				std::cout << "Received " << recvByteCount << " bytes from a " << helper::RECV_BUFFER_LEN << " bytes buffer" << std::endl;
			}
			// get msg from recv buffer
			int bufferCharCount = sizeof(recvBuffer) / sizeof(recvBuffer[0]);
			std::string recvMsg = GetRecvBufferMsg(recvBuffer, bufferCharCount, recvByteCount);
			{
				// lock writing output to console
				std::lock_guard<std::mutex> lock(ioMutex);
				std::cout << "Received msg: " << recvMsg << std::endl;
			}
			// close socket msg
			if (recvMsg == helper::CLOSE_SOCKET_MSG)
			{
				isSocketOpen = false;
				continue;
			}
			// lock writing output to console
			std::lock_guard<std::mutex> lock(ioMutex);
			std::cout << "SERVER: " << recvMsg << std::endl;
		}
		{
			// lock writing output to console
			std::lock_guard<std::mutex> lock(ioMutex);
			std::cout << "Recv loop closed" << std::endl;
		}
		// Check thread state before joining
        if (userInputLoopThread.joinable()) 
		{
			// Wait for the input thread to finish
            userInputLoopThread.join();
		}
	} 
	catch (std::runtime_error e) 
	{
		isSocketOpen = false;
		// lock writing output to console
        std::lock_guard<std::mutex> lock(ioMutex);
		std::cout << "An exception occured: " << e.what() << std::endl;
	}
	// clean up
	try	
	{
		clientSocket.Close();
		std::cout << "Client socket closed" << std::endl;
		CloseWinsockDll();
		std::cout << "Winsock dll closed" << std::endl;
	}
	catch (std::runtime_error e)
	{
		std::cout << "An exception occured on cleanup: " << e.what() << std::endl;
	}
	return 0;
}
