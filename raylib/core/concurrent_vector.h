#pragma once

#include <vector>
#include <mutex>
#include <algorithm>

template<typename T>
class concurrent_vector
{
public:
	void push_back(const T& item) // copy ver
	{
		std::lock_guard<std::mutex> lockGuard(cs);
		std_vector.push_back(item);
	}
	void push_back(T&& item) // move ver
	{
		std::lock_guard<std::mutex> lockGuard(cs);
		std_vector.push_back(item);
	}

	bool contains(const T& item) const
	{
		std::lock_guard<std::mutex> lockGuard(cs);
		for (const T& x : std_vector)
		{
			if (x == item) return true;
		}
		return false;
	}

	// Erase first occurrence of item.
	// @return true if found one and erased it, false otherwise.
	bool erase_first(const T& item)
	{
		std::lock_guard<std::mutex> lockGuard(cs);
		auto it = std::find(std_vector.begin(), std_vector.end(), item);
		if (it != std_vector.end())
		{
			std_vector.erase(it);
			return true;
		}
		return false;
	}
	
private:
	std::vector<T> std_vector;
	std::mutex cs;
};
