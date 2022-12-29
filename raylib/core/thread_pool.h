#pragma once

#include "raylib_types.h"
#include "core/int_types.h"
#include "core/noncopyable.h"

#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <functional>

class ThreadPool;
struct WorkItemParam;

using WorkItemRoutine = std::function<void(const WorkItemParam*)>;

struct ThreadPoolWork
{
	WorkItemRoutine routine;
	void* arg = nullptr;
};

// Parameter for threads in the thread pool
struct PooledThreadParam
{
	PooledThreadParam()
		: threadID(-1)
		, pool(nullptr)
		, started(false)
		, done(false)
	{
	}

	int32             threadID;
	ThreadPool*       pool;
	bool              started;
	bool              done;
};

// Passed to the WorkItemRoutine as a sole parameter
struct WorkItemParam
{
	int32 threadID;
	void* arg;
};

// Not intended for use across several frames.
class ThreadPool : public Noncopyable
{

public:
	RAYLIB_API ThreadPool();
	RAYLIB_API ~ThreadPool();

	RAYLIB_API void Initialize(int32 numWorkerThreads);

	// Do not add any work after Start()
	RAYLIB_API void AddWork(const ThreadPoolWork& workItem);

	RAYLIB_API void Start(bool blocking);
	RAYLIB_API bool IsDone() const;

	// Returns false if no work
	RAYLIB_API bool PopWork(ThreadPoolWork& work);

	RAYLIB_API inline float GetProgress() const
	{
		return 1.0f - (float)(queueIx + 1) / (float)queue.size();
	}

public:
	std::vector<std::thread>                 threads;
	std::vector<PooledThreadParam>           threadParams;

	std::vector<ThreadPoolWork>              queue;
	std::mutex                               queueLock;
	int32                                    queueIx;
};
