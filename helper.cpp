#include <WinSock2.h>
#include <iostream>
#include "helper.h"
#include <stdexcept>

void InitWinsockDll()
{
	// winsock version to use
	WORD wVersion = MAKEWORD(2,2);
	// winsock config data pointer
	WSADATA wsaData; 
	// init winsock dll 
	int res = WSAStartup(wVersion, &wsaData);
	if (res != 0)
	{
		throw std::runtime_error("The winsock dll not found");
	}
}
void CloseWinsockDll()
{
	// close winsock dll
	int res = WSACleanup();
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("The winsock dll not found");
	}
}

bool SendMsg(SOCKET conn, std::string msg, bool showOutput)
{
	int msgByteCount = msg.length();
	int byteSentCount = send(conn, msg.c_str(), msgByteCount, 0);
	// failed to send msg
	if (byteSentCount == SOCKET_ERROR)
	{
		if (showOutput)
		{
			std::cout << "Failed to send msg: " << msg << std::endl;
		}
		return false;
	}
	if (showOutput)
	{
		std::cout << "Sent msg: " << msg << std::endl;
	}
	return true;
}

std::string GetRecvBufferMsg(char recvBuffer[], int bufferCharCount, int recvByteCount)
{
	std::string recvMsg = "";
	int chCount = 0;
	for (int i = 0; i < bufferCharCount; i++)
	{
		char ch = recvBuffer[i];
		recvMsg += ch;
		chCount++;
		if (chCount == recvByteCount)
		{
			break;
		}
	}
	return recvMsg;
}

void CloseSocket(SOCKET conn)
{
	if (conn == INVALID_SOCKET)
	{
		return;
	}
	// close socket
	int res = closesocket(conn);
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to close socket");
	}
}
