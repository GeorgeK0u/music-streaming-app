#pragma once
#include <fstream>

class FileHandler
{
	public:	
		FileHandler(int maxChunkSize);
		void CheckReadFile(const char* fileName);
		char* ReadChunk();
		int GetChunkSize();
		void Release();

	private:
		std::ifstream file;
		int fileSize;
		int maxChunkSize;
		int chunkSize;
};
