#include "server.h"
#include "..\helper.h"
#include <iostream>
#include <stdexcept>

ServerSocket::ServerSocket(const char* ipv4Addr, int port, int maxConnNum)
{
	this->ipv4Addr = ipv4Addr;
	this->port = port;
	this->maxConnNum = maxConnNum;
	this->listenSocket = INVALID_SOCKET;
	this->conn = INVALID_SOCKET;
}

void ServerSocket::Create()
{
	helper::InitWinsockDll();
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
	addr.sin_addr.s_addr = inet_addr(this->ipv4Addr);
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

void ServerSocket::Accept()
{
	// waiting to accept
	this->conn = accept(this->listenSocket, NULL, NULL);
	if (this->conn == INVALID_SOCKET)
	{
		throw std::runtime_error("Accept failed");
	}
	// close listening socket
	if (this->maxConnNum == 1)
	{
		int res = closesocket(this->listenSocket);
		if (res == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to close listening socket");
		}
		this->listenSocket = INVALID_SOCKET;
	}
}

SOCKET ServerSocket::CreateAndAccept()
{
	this->Create();	
	this->Bind();
	this->Listen();
	this->Accept();
	return this->conn;
}

void ServerSocket::Close()
{
	// close conn 
	if (this->conn != INVALID_SOCKET) 
	{
		int res = closesocket(this->conn);
		// failed to close conn
		if (res == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to close listening socket");
		}
		this->conn = INVALID_SOCKET;
	}
	helper::CloseWinsockDll();
}
