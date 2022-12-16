#pragma once

#include <vector>
#include <string>
#include <algorithm>

// No-brainer unoptimal parser for command line arguments.
//
// program.exe arg1 -opt1 value1 arg2 arg3 -opt2 value2
// -> single arguments: arg1, arg2, arg3
// -> pair arguments: (-opt1, value1), (-opt2, value2)
//
// exists("arg1")        = true
// optionExists("-opt1") = true     // Always specify the leading hyphen!
// exists("value1")      = false    // Option value is not an argument
// optionValue("-opt1")  = "value1" // Multiple values are not supported...
//
class ProgramArguments
{
public:
	void init(int argc, char** argv)
	{
		progPath = argv[0];
		for (int i = 1; i < argc; ++i)
		{
			if (argv[i][0] == '-')
			{
				if (i + 1 < argc)
				{
					pairArgs.push_back(std::make_pair(argv[i], argv[i + 1]));
				}
				else
				{
					// Wait, That's Illegal
					pairArgs.push_back(std::make_pair(argv[i], ""));
				}
			}
			else
			{
				singleArgs.push_back(argv[i]);
			}
		}
	}

	std::string programPath() const { return progPath; }

	bool exists(const char* name) const
	{
		return std::find(singleArgs.cbegin(), singleArgs.cend(), name) != singleArgs.cend();
	}

	bool optionExists(const char* optName) const
	{
		for (auto it = pairArgs.cbegin(); it != pairArgs.cend(); ++it)
		{
			if (it->first == optName) return true;
		}
		return false;
	}

	std::string optionValue(const char* optName) const
	{
		for (auto it = pairArgs.cbegin(); it != pairArgs.cend(); ++it)
		{
			if (it->first == optName) return it->second;
		}
		return "";
	}

	std::string progPath;
	std::vector<std::string> singleArgs;
	std::vector<std::pair<std::string, std::string>> pairArgs;
};
