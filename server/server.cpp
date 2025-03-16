#include "server.h"
#include "..\helper.h"
#include <iostream>
#include <stdexcept>

ListenSocket::ListenSocket()
{
	this->listenSocket = INVALID_SOCKET;
}

void ListenSocket::Create()
{
	// init winsock dll
	helper::InitWinsockDll();
	// create listening socket
	this->listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// failed to create listening socket
	if (this->listenSocket == INVALID_SOCKET)
	{
		throw std::runtime_error("Failed to create listening socket");
	}
}

void ListenSocket::Bind()
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

void ListenSocket::Listen()
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

SOCKET ListenSocket::Accept()
{
	// waiting to accept
	SOCKET conn = accept(this->listenSocket, NULL, NULL);
	// accept failed
	if (conn == INVALID_SOCKET)
	{
		throw std::runtime_error("Accept failed");
	}
	return conn;
}

void ListenSocket::CreateAndListen(const char* ipv4Addr, int port, int maxConnNum)
{
	this->ipv4Addr = ipv4Addr;
	this->port = port;
	this->maxConnNum = maxConnNum;
	this->Create();	
	this->Bind();
	this->Listen();
}

void ListenSocket::Close()
{
	// close listen socket
	if (this->listenSocket == INVALID_SOCKET)
	{
		return;
	}
	int res = closesocket(this->listenSocket);
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to close listening socket");
	}
	std::cout << "Listening socket memory released" << std::endl;
	this->listenSocket = INVALID_SOCKET;
}
