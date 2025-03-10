#include <WinSock2.h>
#include "helper.h"
#include <stdexcept>

const char* helper::CLOSE_SOCKET_MSG = "CloseSocket";
const char* helper::START_STREAMING_MSG = "StartStreaming";
const char* helper::STREAMING_FINISHED_MSG = "StreamingFinished";
const char* helper::SEND_CHUNK_MSG = "SendChunk";

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

void helper::CloseSocket(SOCKET conn)
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

void helper::CloseWinsockDll()
{
	// close winsock dll
	int res = WSACleanup();
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("The winsock dll not found");
	}
}
