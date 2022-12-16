#pragma once

void StartLogThread();
void WaitForLogThread();

void LOG(const char* format, ...);
