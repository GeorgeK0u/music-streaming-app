#pragma once
#include <WinSock2.h>

class ServerSocket
{
	public:
		ServerSocket(const char* arr, int port, int maxConnNum);
		SOCKET CreateAndAccept();
		void Close();

	private:
		SOCKET listenSocket;
		SOCKET conn;
		const char* ipv4Addr;
		int port;
		int maxConnNum;
		void Create();
		void Bind();
		void Listen();
		void Accept();	
};
