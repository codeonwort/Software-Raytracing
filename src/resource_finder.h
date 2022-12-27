#pragma once

#include <vector>
#include <string>

// Given a path, searches for a matching full path.
class ResourceFinder
{

public:
	static ResourceFinder& Get();

	// Register a base directory.
	void AddDirectory(const std::string& directory);

	// Find a valid path that starts from one of base directories.
	// Returns an empty string if not found.
	std::string Find(const std::string& subpath);

private:
	ResourceFinder();

	std::vector<std::string> directories;

};
