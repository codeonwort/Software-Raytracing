#include "thread_pool.h"
#include <assert.h>

static void* pooledThreadMain(void* _param)
{
	PooledThreadParam* param = reinterpret_cast<PooledThreadParam*>(_param);
	int32 threadID           = param->threadID;
	ThreadPool* pool         = param->pool;

	//log("Thread %d started to work", threadID);

	bool hasWork = true;

	while(hasWork)
	{
		ThreadPoolWork work;
		hasWork = pool->PopWork(work);

		if(hasWork)
		{
			WorkItemParam param;
			param.threadID = threadID;
			param.arg      = work.arg;

			work.routine(&param);
		}
	}

	//log("Thread %d has finished", threadID);
	param->done = true;

	return 0;
}

ThreadPool::ThreadPool()
	: queueIx(-1)
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::Initialize(int32 numWorkerThreads)
{
	threads.resize(numWorkerThreads);
	threadParams.resize(numWorkerThreads);

	for(int32 i=0; i<numWorkerThreads; ++i)
	{
		threadParams[i].threadID = i;
		threadParams[i].pool = this;
	}
}

void ThreadPool::AddWork(const ThreadPoolWork& workItem)
{
	queue.push_back(workItem);
}

void ThreadPool::Start(bool blocking)
{
	queueIx = (int32)queue.size() - 1;

	int32 n = (int32)threads.size();

	for(int32 i=0; i<n; ++i)
	{
		threads[i] = std::thread(pooledThreadMain, (void*)&threadParams[i]);
		threads[i].detach();
		threadParams[i].started = true;
	}

	if(blocking)
	{
		for(int32 i=0; i<n; ++i)
		{
			threads[i].join();
		}
	}
}

bool ThreadPool::IsDone() const
{
	bool done = true;
	for(auto i=0u; i<threadParams.size(); ++i)
	{
		done = done && threadParams[i].done;
	}
	return done;
}

bool ThreadPool::PopWork(ThreadPoolWork& work)
{
	bool ret = true;

	queueLock.lock();

	if(queueIx < 0)
	{
		ret = false;
	}
	else
	{
		work = queue[queueIx];
		--queueIx;
	}

	queueLock.unlock();

	return ret;
}
