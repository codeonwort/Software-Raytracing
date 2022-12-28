#include "log.h"
#include "core/int_types.h"
#include "core/assertion.h"

#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>

#include <stdio.h>
#include <stdarg.h>


static bool logThreadStarted = false;
static bool logThreadPendingKill = false;
static std::mutex log_mutex;
static std::list<std::string> logQueue;


void StartLogThread()
{
	if (logThreadStarted)
	{
		return;
	}
	logThreadStarted = true;

	void LogMain();
	std::thread logThread(LogMain);
	logThread.detach();
}

void FlushLogThread()
{
	CHECK(logThreadStarted);
	while (logQueue.size() > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void WaitForLogThread()
{
	CHECK(logThreadStarted);
	logThreadPendingKill = true;

	while (logQueue.size() > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void LOG(const char* format, ...)
{
	if (logThreadPendingKill)
	{
		return;
	}

	char buffer[1024];

	va_list ap;
	va_start(ap, format);
	vsprintf_s(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	log_mutex.lock();
	logQueue.emplace_back(buffer);
	log_mutex.unlock();
}

void LogMain()
{
	while (!logThreadPendingKill)
	{
		int32 n = std::min((int32)logQueue.size(), 1000);
		while (n --> 0)
		{
			printf("%s\n", logQueue.front().data());
			logQueue.pop_front();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Cleanup
	int32 n = (int32)logQueue.size();
	while (n --> 0)
	{
		printf("%s\n", logQueue.front().data());
		logQueue.pop_front();
	}
}
