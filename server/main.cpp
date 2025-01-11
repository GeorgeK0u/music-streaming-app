#include "server.h"
#include <iostream>
#include <stdexcept>

int main()
{
	ServerSocket conn = ServerSocket();
	try 
	{
		conn.InitWinsockDll();			
		conn.Create();
		conn.Bind("192.168.2.105", 1234);
		int maxConnNum = 1;
		conn.Listen(maxConnNum);
		conn.Accept();
	} 
	catch (std::runtime_error e) 
	{
		std::cout << "An exception occured: " << e.what() << std::endl;
		return -1;	
	}
	conn.Close();
	conn.CloseWinsockDll();
	return 0;
}
