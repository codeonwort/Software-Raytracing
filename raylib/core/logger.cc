#include "logger.h"
#include "core/int_types.h"
#include "core/assertion.h"

#include <list>
#include <mutex>
#include <thread>
#include <chrono>
#include <algorithm>

#include <stdio.h>
#include <stdarg.h>

static bool bLogThreadStarted = false;
static bool bLogThreadPendingKill = false;
static std::mutex log_mutex;
static std::list<std::string> logQueue;

namespace Logger
{

	void StartLogThread()
	{
		if (bLogThreadStarted)
		{
			return;
		}
		bLogThreadStarted = true;

		void LogMain();
		std::thread logThread(LogMain);
		logThread.detach();
	}

	void FlushLogThread()
	{
		CHECK(bLogThreadStarted);
		while (logQueue.size() > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void KillAndWaitForLogThread()
	{
		CHECK(bLogThreadStarted);
		bLogThreadPendingKill = true;

		while (logQueue.size() > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

}

void LOG(const char* format, ...)
{
	if (bLogThreadPendingKill)
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
	while (!bLogThreadPendingKill)
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
