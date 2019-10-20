#pragma once

#include "type.h"
#include "image.h"
#include <fstream>

// Wrapper for std::iostream
class File
{

public:
	void Open(const String& path);
	void Seek(uint32 position);
	void Write(void* buffer, uint32 size);
	void Close();

private:
	std::fstream fs;

};

///////////////////////////////////////////////////////

void WriteBitmap(const Image2D& image, const char* filepath);

