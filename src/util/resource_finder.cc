#include "resource_finder.h"
#include "../platform/platform.h"
#include "../log.h"

#include <assert.h>

////////////////////////////////////////////////////////
// Platform-specific
#if PLATFORM_WINDOWS

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
static inline bool FileExists(const std::string& path)
{
	return PathFileExistsA(path.c_str()) == TRUE ? true : false;
}

#else

#error "Implement this"
static inline bool FileExists(const std::string& path)
{
	static_assert(false, "Implement this");
	return false;
}

#endif
////////////////////////////////////////////////////////

ResourceFinder& ResourceFinder::Get()
{
	// C++11 guarantees this is thread-safe
	static ResourceFinder instance;
	return instance;
}

ResourceFinder::ResourceFinder()
{
	directories.push_back(""); // For absolute path
	directories.push_back("./");
	directories.push_back(SOLUTION_DIR);
}

void ResourceFinder::AddDirectory(const std::string& directory)
{
	auto last = directory.at(directory.size() - 1);
	assert(last == '/' || last == '\\');
	directories.push_back(directory);
}

std::string ResourceFinder::Find(const std::string& subpath)
{
	if (subpath.size() == 0)
	{
		return "";
	}

	for (const auto& dir : directories)
	{
		auto fullpath = dir + subpath;
		if (FileExists(fullpath))
		{
			return fullpath;
		}
	}

	log("%s: not found: %s", __FUNCTION__, subpath.data());

	return "";
}
