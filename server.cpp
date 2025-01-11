#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdexcept>

int main()
{
	SOCKET serverSocket = INVALID_SOCKET;
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
		serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		// failed to create socket
		if (serverSocket == INVALID_SOCKET)
		{
			throw std::runtime_error("Failed to create server socket");
		}
		// create socket address instance for bind
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr("192.168.2.105");
		addr.sin_port = htons(1234);
		// bind socket
		int bindOut = bind(serverSocket, (SOCKADDR*)&addr, sizeof(addr));
		// failed to bind socket
		if (bindOut == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to bind server socket");
		}
		// set socket to listen
		int maxConnNum = 1;
		int listenOut = listen(serverSocket, maxConnNum);
		// failed to bind socket
		if (listenOut == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to set server socket for listening");
		}
		std::cout << "Listening..." << std::endl;
		// accept
		int acceptOut = accept(serverSocket, NULL, NULL);
		if (acceptOut == SOCKET_ERROR)
		{
			throw std::runtime_error("Accept failed");
		}
	} 
	catch (std::runtime_error e)
	{
		std::cout << "An exception occured: " << e.what() << std::endl;
	}
	// close socket
	if (serverSocket != INVALID_SOCKET)
	{
		closesocket(serverSocket);
		std::cout << "Server socket closed" << std::endl;
	}
	// close winsock dll
	WSACleanup();
	std::cout << "Winsock dll closed" << std::endl;
	return 0;
}
