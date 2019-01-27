#include "file.h"
#include "log.h"
#include <memory.h>

void File::Open(const String& path)
{
	fs.open(path.GetData(),
		std::fstream::in
		| std::fstream::out
		| std::fstream::binary
		| std::fstream::trunc);
}

void File::Seek(uint32 position)
{
	fs.seekg(position);
}

void File::Write(void* buffer, uint32 size)
{
	fs.write((char*)buffer, size);
}

void File::Close()
{
	fs.close();
}

/////////////////////////////////////////////////////////////////////////

#define BI_RGB 0x0000
using WORD  = uint16;
using DWORD = uint32;
using LONG  = int32;

#pragma pack(push)
#pragma pack(1)
struct BITMAPFILEHEADER
{
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(1)
struct BITMAPINFOHEADER
{
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
};
#pragma pack(pop)

void WriteBitmap(const HDRImage& image, const char* filepath)
{
	File file;
	file.Open(String(filepath));

	log("write bitmap (width:%d, height:%d)", image.GetWidth(), image.GetHeight());

	uint32 bufferSize = 3 * image.GetWidth() * image.GetHeight();

	// Write
	BITMAPFILEHEADER fileHeader;
	fileHeader.bfType      = 'MB';
	fileHeader.bfSize      = sizeof(BITMAPFILEHEADER) + bufferSize;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER infoHeader;
	memset(&infoHeader, 0, sizeof(infoHeader));
	infoHeader.biSize        = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth       = image.GetWidth();
	infoHeader.biHeight      = image.GetHeight();
	infoHeader.biPlanes      = 1;
	infoHeader.biBitCount    = 24;
	infoHeader.biCompression = BI_RGB;
	infoHeader.biSizeImage   = 0;

	uint8* buffer = new uint8[bufferSize];
	uint8* it     = buffer;

	for(int y = image.GetHeight() - 1; y >= 0; --y)
	{
		for(int x = 0; x < image.GetWidth(); ++x)
		{
			Pixel px = image.GetPixel(x, y);
			uint8 r  = (int32)(px.r * 255.0f) & 0xff;
			uint8 g  = (int32)(px.g * 255.0f) & 0xff;
			uint8 b  = (int32)(px.b * 255.0f) & 0xff;
			*(it++) = b;
			*(it++) = g;
			*(it++) = r;
		}
	}

	file.Write(&fileHeader, sizeof(fileHeader));
	file.Write(&infoHeader, sizeof(infoHeader));
	file.Write(buffer,      bufferSize);

	delete buffer;

	file.Close();
}

