#pragma once

class helper
{
	public:
		static const int RECV_BUFFER_LEN = 128 * 1024;
		static const char CLOSE_SOCKET_KEY = 'X';
		static const char* CLOSE_SOCKET_MSG;
		static const char* START_STREAMING_MSG;
		static const char* STREAMING_FINISHED_MSG;
		static const char* SEND_CHUNK_MSG;
		static void GetSocketConfig(const char*& ipv4Addr, int& port);
		static void InitWinsockDll();
		static void SendMsg(SOCKET conn, const char* msg);
		static void SendChunkMsg(SOCKET conn, const char* msg, int msgByteCount);
		static void GetRecvMsg(char recvBuffer[], int recvByteCount, char recvMsg[]);
		static void CloseSocket(SOCKET& conn);
		static void CloseWinsockDll();
	private:
		static const char* ipv4Addr;
		static int ipv4AddrLength;
		static int port;
		static void ReadSocketConfig();
};
