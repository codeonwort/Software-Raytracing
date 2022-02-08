#pragma once

void StartLogThread();
void WaitForLogThread();

void log(const char* format, ...);
