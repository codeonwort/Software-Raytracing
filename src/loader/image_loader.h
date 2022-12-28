#pragma once

#include "core/noncopyable.h"
#include "render/image.h"

#include "raylib_types.h"

#include <memory>

class ImageLoader : public Noncopyable
{

public:
	static void Initialize();
	static void Destroy();

	// Simple load. Create an ImageLoader instance for advanced functionality.
	static bool SyncLoad(const char* filepath, Image2D& outImage);
	static bool SyncLoad(const char* filepath, std::shared_ptr<Image2D>& outImage);

public:
	explicit ImageLoader();
	~ImageLoader();

	bool LoadSynchronous(const char* filepath, Image2D& outImage);

	/*
	 * #todo-imageloader: async load
	 * ThreadHandle LoadAsync(std::vector<const char*> filepathArray);
	 * AsyncLoadProgress GetProgress();
	 */
	
};

void WriteImageToDisk(const Image2D& image, const char* filepath, EImageFileType fileType);
