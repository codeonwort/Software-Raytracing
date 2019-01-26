#include "image.h"
#include "file.h"
#include "log.h"

int main(int argc, char** argv)
{
	log("raytracing study");

	log("generate a test image");

	const int32 width = 1024;
	const int32 height = 512;
	HDRImage image(width, height, 0x123456);
	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			Pixel px;
			px.r = (float)x / float(width);
			px.g = (float)y / float(height);
			image.SetPixel(x, y, px);
		}
	}

	WriteBitmap(image, "test.bmp");

	log("image has been written as bitmap");

	return 0;
}

