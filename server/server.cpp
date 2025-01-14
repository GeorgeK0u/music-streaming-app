#include <WinSock2.h>
#include <iostream>
#include "server.h"
#include <stdexcept>

ServerSocket::ServerSocket(std::string ipv4Addr, int port, int maxConnNum)
{
	this->listenSocket = INVALID_SOCKET;
	this->ipv4Addr = ipv4Addr;
	this->port = port;
	this->maxConnNum = maxConnNum;
}

void ServerSocket::Create()
{
	// create listening socket
	this->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// failed to create listening socket
	if (this->listenSocket == INVALID_SOCKET)
	{
		throw std::runtime_error("Failed to create listening socket");
	}
}

void ServerSocket::Bind()
{
	// listening socket address struct
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(this->ipv4Addr.c_str());
	addr.sin_port = htons(this->port);
	// bind listening socket
	int res = bind(this->listenSocket, (SOCKADDR*)&addr, sizeof(addr));
	// failed to bind listening socket
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to bind listening socket");
	}
}

void ServerSocket::Listen()
{
	// set socket to listen
	int res = listen(this->listenSocket, this->maxConnNum);
	// failed to set socket to listen
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to set socket to listen");
	}
	std::cout << "Listening..." << std::endl;
}

SOCKET ServerSocket::Accept()
{
	// waiting to accept
	SOCKET conn = accept(this->listenSocket, NULL, NULL);
	if (conn == INVALID_SOCKET)
	{
		throw std::runtime_error("Accept failed");
	}
	std::cout << "Accepted client" << std::endl;
	// close listening socket
	if (this->maxConnNum == 1)
	{
		int res = closesocket(this->listenSocket);
		if (res == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to close listening socket");
		}
	}
	return conn;
}

SOCKET ServerSocket::InitAndAccept()
{
	Create();	
	Bind();
	Listen();
	return Accept();
}

void ServerSocket::Close()
{
	if (this->listenSocket == INVALID_SOCKET)
	{
		return;
	}
	// close listening socket
	int res = closesocket(this->listenSocket);
	// failed to close listening socket
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to close listening socket");
	}
	std::cout << "Listening socket closed" << std::endl;
}
