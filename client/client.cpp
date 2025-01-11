#include "client.h"
#include <WS2tcpip.h>
#include <iostream>
#include <stdexcept>

ClientSocket::ClientSocket()
{
	conn = INVALID_SOCKET;
}

void ClientSocket::InitWinsockDll()
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

void ClientSocket::Create()
{
	// create socket
	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// failed to create socket
	if (conn == INVALID_SOCKET)
	{
		throw std::runtime_error("Failed to create client socket");
	}
}

void ClientSocket::Connect(const char* ipv4Addr, int port)
{
	// create socket address instance for connect
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipv4Addr);
	addr.sin_port = htons(port);
	// connect
	int connOut = connect(conn, (SOCKADDR*)&addr, sizeof(addr));
	// failed to connect
	if (connOut == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to connect");
	}
	std::cout << "Client connected successfully!" << std::endl;
}

void ClientSocket::Close()
{
	if (conn == INVALID_SOCKET)
	{
		return;
	}
	// close socket
	closesocket(conn);
	std::cout << "Client socket closed" << std::endl;
}

void ClientSocket::CloseWinsockDll()
{
	// close winsock dll
	WSACleanup();
	std::cout << "Winsock dll closed" << std::endl;
}
