#pragma once
#include <WinSock2.h>

class ServerSocket
{
	public:
		ServerSocket();
		void InitWinsockDll();
		void Create();
		void Bind(const char* ipv4Addr, int port);
		void Listen(int maxConnNum);
		void Accept();	
		void Close();
		void CloseWinsockDll();

	private:
		SOCKET conn;
};
