#pragma once
#include <WinSock2.h>

class ClientSocket
{
	public:
		ClientSocket();
		void InitWinsockDll();
		void Create();
		void Connect(const char* ipv4Addr, int port);
		void Close(); 
		void CloseWinsockDll();

	private:	
		SOCKET conn;
};
