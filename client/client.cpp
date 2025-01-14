#include <WinSock2.h>
#include <string>
#include "client.h"
#include <stdexcept>

ClientSocket::ClientSocket(std::string ipv4Addr, int port)
{
	this->conn = INVALID_SOCKET;
	this->ipv4Addr = ipv4Addr;
	this->port = port;
}

void ClientSocket::Create()
{
	// create socket
	this->conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// failed to create socket
	if (this->conn == INVALID_SOCKET)
	{
		throw std::runtime_error("Failed to create client socket");
	}
}

void ClientSocket::Connect()
{
	// create socket address instance for connect
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(this->ipv4Addr.c_str());
	addr.sin_port = htons(this->port);
	// connect
	int res = connect(this->conn, (SOCKADDR*)&addr, sizeof(addr));
	// failed to connect
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to connect");
	}
}

SOCKET ClientSocket::CreateAndConnect()
{
	this->Create();
	this->Connect();
	return conn;
}

void ClientSocket::Close()
{
	if (this->conn == INVALID_SOCKET)
	{
		return;
	}
	// close socket
	int res = closesocket(this->conn);
	if (res == SOCKET_ERROR)
	{
		throw std::runtime_error("Failed to close client socket"); 
	}
}
