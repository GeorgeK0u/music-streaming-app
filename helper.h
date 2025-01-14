#pragma once

namespace helper
{
	const int RECV_BUFFER_LEN = 512;
	const std::string CLOSE_SOCKET_MSG = "CloseSocket";
	const std::string CLOSE_SOCKET_KEY = "x";
}
void InitWinsockDll();
void CloseWinsockDll();
bool SendMsg(SOCKET conn, std::string msg, bool showOutput=false);
std::string GetRecvBufferMsg(char recvBuffer[], int bufferCharCount, int recvByteCount); 
void CloseSocket(SOCKET conn);
