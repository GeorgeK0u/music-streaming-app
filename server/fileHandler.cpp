#include "fileHandler.h"
#include <iostream>
#include <stdexcept>
#define WAVE_FORMAT_PCM 1

FileHandler::FileHandler(int maxChunkSize)
{
	this->maxChunkSize = maxChunkSize;
}

struct WaveHeader
{
	// riff chunk
	char fileTypeBlocId[4];
	long fileSize;
	char fileFormatId[4];
	// fmt chunk
	char formatBlockId[4];
	long formatSize;
	short audioFormat;
	short nChannels;
	long samplesPerSec;
	long bytePerSec;
	short bytePerBloc;
	short bitsPerSample;
	// data chunk
	char dataBlocId[4];
	long dataSize;
} waveHeader;

void FileHandler::CheckReadFile(const char* fileName)
{
	// open file for binary reading
	this->file.open(fileName, std::ios::binary);
	// failed to open wav file
	if (!this->file.is_open())
	{
		throw std::runtime_error("Failed to open wav file");
	}
	// validate wav file headers
	this->file.read(reinterpret_cast<char*>(&waveHeader), sizeof(WaveHeader));
	if (std::string(waveHeader.fileTypeBlocId, 4) != "RIFF" || std::string(waveHeader.fileFormatId, 4) != "WAVE")
	{
		throw std::runtime_error("Invalid WAV file headers");
	}
	if (waveHeader.audioFormat != WAVE_FORMAT_PCM)
	{
		throw std::runtime_error("Non PCM audio format");
	}
	// get file size
	this->fileSize = waveHeader.fileSize;
}

char* FileHandler::ReadChunk()
{
	// calculate remaining size
	int remainingSize = this->fileSize - this->file.tellg();
	// chunk size
	this->chunkSize = (remainingSize >= this->maxChunkSize) ? this->maxChunkSize : remainingSize;
	// streaming finished
	if (this->chunkSize == 0)
	{
		this->file.close();
		return nullptr;
	}
	std::cout << "Chunk size: " << this->chunkSize << std::endl;
	// read chunk
	char* chunkDataArrPtr = new char[this->chunkSize];
	file.read(chunkDataArrPtr, this->chunkSize);
	return chunkDataArrPtr;
}

int FileHandler::GetChunkSize()
{
	return this->chunkSize;
}

void FileHandler::Release()
{
	if (this->file.is_open())
	{
		this->file.close();
	}
}
