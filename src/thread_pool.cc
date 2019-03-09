#include "thread_pool.h"
#include "log.h"
#include <assert.h>


static void* pooledThreadMain(void* _param)
{
	PooledThreadParam* param = reinterpret_cast<PooledThreadParam*>(_param);
	int32 threadID           = param->threadID;
	ThreadPool* pool         = param->pool;

	log("Thread %d is pending...", threadID);

	while(param->started == false)
	{
		pthread_yield();
	}

	log("Thread %d started to work", threadID);

	while(true)
	{
		ThreadPoolWork work;
		if(pool->PopWork(work))
		{
			WorkItemParam param;
			param.threadID = threadID;
			param.arg      = work.arg;

			work.routine(&param);
		}
		else
		{
			break;
		}
	}

	log("Thread %d has finished", threadID);
	param->done = true;
}

ThreadPool::ThreadPool()
	: queueIx(-1)
{
}

ThreadPool::~ThreadPool()
{
}

void ThreadPool::initialize(int32 numWorkerThreads)
{
	threads.resize(numWorkerThreads);
	threadParams.resize(numWorkerThreads);

	for(int32 i=0; i<numWorkerThreads; ++i)
	{
		threadParams[i].threadID = i;
		threadParams[i].pool = this;

		int ret = pthread_create(&threads[i], NULL, pooledThreadMain, (void*)&threadParams[i]);
		assert(ret == 0);
	}
}

void ThreadPool::addWork(const ThreadPoolWork& workItem)
{
	queue.push_back(workItem);
}

void ThreadPool::Start(bool blocking)
{
	queueIx = (int32)queue.size() - 1;

	int32 n = (int32)threads.size();

	for(int32 i=0; i<n; ++i)
	{
		if(blocking == false)
		{
			pthread_detach(threads[i]);
		}
		threadParams[i].started = true;
	}

	if(blocking)
	{
		for(int32 i=0; i<n; ++i)
		{
			pthread_join(threads[i], NULL);
		}
	}
}

bool ThreadPool::Done() const
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

