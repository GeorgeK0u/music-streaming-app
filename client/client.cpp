#include "client.h"
#include "..\helper.h"
#include <stdexcept>

ClientSocket::ClientSocket(const char* ipv4Addr, int port)
{
	this->conn = INVALID_SOCKET;
	this->ipv4Addr = ipv4Addr;
	this->port = port;
}

void ClientSocket::Create()
{
	helper::InitWinsockDll();
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
	addr.sin_addr.s_addr = inet_addr(this->ipv4Addr);
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
	return this->conn;
}

void ClientSocket::Close()
{
	if (this->conn != INVALID_SOCKET)
	{
		// close socket
		int res = closesocket(this->conn);
		if (res == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to close client socket"); 
		}
		this->conn = INVALID_SOCKET;
	}
	helper::CloseWinsockDll();
}
