#pragma once

#include "core/int_types.h"

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
class ThreadPool
{

public:
	ThreadPool();
	~ThreadPool();

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

	void Initialize(int32 numWorkerThreads);

	// Do not add any work after Start()
	void AddWork(const ThreadPoolWork& workItem);

	void Start(bool blocking);
	bool IsDone() const;

	// Returns false if no work
	bool PopWork(ThreadPoolWork& work);

	inline float GetProgress() const
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
