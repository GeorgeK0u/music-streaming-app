#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdexcept>

int main()
{
	SOCKET clientSocket = INVALID_SOCKET;
	try 
	{
		// winsock config data pointer
		WSADATA wsaData; 
		// winsock version to use
		WORD wVersion = MAKEWORD(2,2);
		// run winsock dll 
		int dllOut = WSAStartup(wVersion, &wsaData);
		if (dllOut != 0)
		{
			throw std::runtime_error("The winsock dll not found");
		}
		// create socket
		clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		// failed to create socket
		if (clientSocket == INVALID_SOCKET)
		{
			throw std::runtime_error("Failed to create client socket");
		}
		// create socket address instance for connect
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("192.168.2.105");
		addr.sin_port = htons(1234);
		// connect
		int connectOut = connect(clientSocket, (SOCKADDR*)&addr, sizeof(addr));
		if (connectOut == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to connect");
		}
		std::cout << "Client connected successfully!" << std::endl;
	}
	catch (std::runtime_error e)
	{
		std::cout << "An exception occured: " << e.what() << std::endl;
	}
	// close socket
	if (clientSocket != INVALID_SOCKET)
	{
		closesocket(clientSocket);
		std::cout << "Client socket closed" << std::endl;
	}
	// close winsock dll
	WSACleanup();
	std::cout << "Winsock dll closed" << std::endl;
	return 0;
}
