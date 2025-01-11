#include "server.h"
#include <WS2tcpip.h>	
#include <iostream>
#include <stdexcept>

ServerSocket::ServerSocket()
{
	conn = INVALID_SOCKET;
}

void ServerSocket::InitWinsockDll()
{
	// winsock version to use
	WORD wVersion = MAKEWORD(2,2);
	// winsock config data pointer
	WSADATA wsaData; 
	// init winsock dll 
	int initDllOut = WSAStartup(wVersion, &wsaData);
	if (initDllOut != 0)
	{
		throw std::runtime_error("The winsock dll not found");
	}
}

void ServerSocket::Create()
{
	// create socket
	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// failed to create socket
	if (conn == INVALID_SOCKET)
	{
		throw std::runtime_error("Failed to create server socket");
	}
}

void ServerSocket::Bind(const char* ipv4Addr, int port)
{
	// create socket address instance for bind
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipv4Addr);
	addr.sin_port = htons(port);
	// bind socket
	int bindOut = bind(conn, (SOCKADDR*)&addr, sizeof(addr));
	// failed to bind socket
	if (bindOut == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to bind server socket");
	}
}

void ServerSocket::Listen(int maxConnNum)
{
	// set socket to listen
	int listenOut = listen(conn, maxConnNum);
	// failed to bind socket
	if (listenOut == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to set server socket for listening");
	}
	std::cout << "Listening..." << std::endl;
}

void ServerSocket::Accept()
{
	// waiting to accept
	int acceptOut = accept(conn, NULL, NULL);
	if (acceptOut == SOCKET_ERROR)
	{
		throw std::runtime_error("Accept failed");
	}
	std::cout << "Accepted client" << std::endl;
}

void ServerSocket::Close()
{
	if (conn == INVALID_SOCKET)
	{
		return;
	}
	// close socket
	closesocket(conn);
	std::cout << "Server socket closed" << std::endl;
}

void ServerSocket::CloseWinsockDll()
{
	// close winsock dll
	WSACleanup();
	std::cout << "Winsock dll closed" << std::endl;
}
