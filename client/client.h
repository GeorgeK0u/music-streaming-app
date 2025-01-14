#pragma once

class ClientSocket
{
	public:
		ClientSocket(std::string ipv4Addr, int port);
		SOCKET CreateAndConnect();
		void Close(); 

	private:	
		SOCKET conn;
		std::string ipv4Addr;
		int port;
		void Create();
		void Connect();
};
