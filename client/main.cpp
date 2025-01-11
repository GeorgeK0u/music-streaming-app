#include "client.h"
#include <iostream>
#include <stdexcept>

int main()
{
	ClientSocket conn = ClientSocket();
	try 
	{
		conn.InitWinsockDll();
		conn.Create();
		conn.Connect("192.168.2.105", 1234);
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
