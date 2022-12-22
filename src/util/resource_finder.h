#pragma once

#include "core/noncopyable.h"

#include <vector>
#include <string>

// Given a path, searches for a matching full path.
class ResourceFinder : public Noncopyable
{

public:
	static ResourceFinder& Get();

	// Register a directory
	void AddDirectory(const std::string& directory);

	// Returns empty string if not found
	std::string Find(const std::string& subpath);

private:
	ResourceFinder();

	std::vector<std::string> directories;

};
