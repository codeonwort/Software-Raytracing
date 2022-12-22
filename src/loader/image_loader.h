#pragma once

#include "core/noncopyable.h"
#include "render/image.h"
#include <memory>

// #todo-wip: Move to .cc
#pragma warning(push)
#pragma warning(disable: 4819)
#include "FreeImage.h"
#pragma warning(pop)

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

enum class EImageFileType
{
	Bitmap,
	Jpg,
	Png
};

void WriteImageToDisk(const Image2D& image, const char* filepath, EImageFileType fileType);
