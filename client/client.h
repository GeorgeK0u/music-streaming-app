#pragma once
#include <WinSock2.h>

class ClientSocket
{
	public:
		ClientSocket(const char* ipv4Addr, int port);
		SOCKET CreateAndConnect();
		void Close(); 

	private:	
		SOCKET conn;
		const char* ipv4Addr;
		int port;
		void Create();
		void Connect();
};
