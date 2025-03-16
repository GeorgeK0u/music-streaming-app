#include <WinSock2.h>
#include "helper.h"
#include <stdexcept>
#include <fstream>
#include <string>

const char* helper::CLOSE_SOCKET_MSG = "CloseSocket";
const char* helper::START_STREAMING_MSG = "StartStreaming";
const char* helper::STREAMING_FINISHED_MSG = "StreamingFinished";
const char* helper::SEND_CHUNK_MSG = "SendChunk";
const char* helper::ipv4Addr = nullptr;
int helper::ipv4AddrLength = 0;
int helper::port = 0;

void helper::GetSocketConfig(const char*& ipv4Addr, int& port)
{
	if (helper::ipv4Addr == nullptr)
	{
		ReadSocketConfig();
	}
	ipv4Addr = new char[helper::ipv4AddrLength];
	memcpy((void*)ipv4Addr, helper::ipv4Addr, helper::ipv4AddrLength);
	port = helper::port;
}

void helper::ReadSocketConfig()
{
	std::ifstream file;
	file.open("../socket-config.txt");
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open socket-config txt file");
	}
	std::string ipv4Addr, portStr;
	getline(file, ipv4Addr);
	// +1 for the null byte
	helper::ipv4AddrLength = ipv4Addr.length()+1;
	helper::ipv4Addr = new char[helper::ipv4AddrLength];
	memcpy((void*)helper::ipv4Addr, ipv4Addr.c_str(), helper::ipv4AddrLength);
	getline(file, portStr);
	file.close();
	int port = std::stoi(portStr);
	helper::port = port;
}

void helper::InitWinsockDll()
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

void helper::SendMsg(SOCKET conn, const char* msg)
{
	int msgByteCount = strlen(msg);
	SendChunkMsg(conn, msg, msgByteCount);
}
void helper::SendChunkMsg(SOCKET conn, const char* msg, int msgByteCount)
{
	int byteSentCount = send(conn, msg, msgByteCount, 0);
	// failed to send msg
	if (byteSentCount == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to send msg");
	}
}

void helper::GetRecvMsg(char recvBuffer[], int recvByteCount, char recvMsg[])
{
	for (int i = 0; i < recvByteCount; i++)
	{
		char ch = recvBuffer[i];
		recvMsg[i] = ch;
	}
}

void helper::CloseSocket(SOCKET& conn)
{
	if (conn == INVALID_SOCKET)
	{
		return;
	}
	// close socket
	int res = closesocket(conn);
	// failed to close socket
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to close socket");
	}
	conn = INVALID_SOCKET;
}

void helper::CloseWinsockDll()
{
	// close winsock dll
	int res = WSACleanup();
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("The winsock dll not found");
	}
	// delete ip dynamic memory
	delete[] helper::ipv4Addr;
}
