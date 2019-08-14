#pragma once

#include "image.h"

#include "FreeImage.h"

class ImageLoader
{

public:
	static void Initialize();
	static void Destroy();

	// Simple load. Create an ImageLoader instance for advanced functionality.
	static void SyncLoad(const char* filepath, HDRImage& outImage);

public:
	explicit ImageLoader();
	~ImageLoader();

	ImageLoader(const ImageLoader&) = delete;
	ImageLoader& operator=(const ImageLoader&) = delete;

	void LoadSynchronous(const char* filepath, HDRImage& outImage);

	/*
	 * #todo: async load
	 * ThreadHandle LoadAsync(std::vector<const char*> filepathArray);
	 * AsyncLoadProgress GetProgress();
	 */

};
