#pragma once

void StartLogThread();
void FlushLogThread();
void WaitForLogThread();

void LOG(const char* format, ...);
