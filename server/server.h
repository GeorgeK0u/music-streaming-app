#pragma once
#include <WinSock2.h>

class ListenSocket
{
	public:
		ListenSocket();
		void CreateAndListen(const char* arr, int port, int maxConnNum);
		SOCKET Accept();	
		void Close();

	private:
		SOCKET listenSocket;
		const char* ipv4Addr;
		int port;
		int maxConnNum;
		void Create();
		void Bind();
		void Listen();
};
