#pragma once

class ServerSocket
{
	public:
		ServerSocket(std::string ipv4Addr, int port, int maxConnNum);
		SOCKET InitAndAccept();
		void Close();

	private:
		SOCKET listenSocket;
		std::string ipv4Addr;
		int port;
		int maxConnNum;
		void Create();
		void Bind();
		void Listen();
		SOCKET Accept();	
};
