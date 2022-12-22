#pragma once

// Main header for raytracing library.

// #todo-raylib:
// 1. Move all raytracing logic to this project.
// 2. Rename 'src' as something like 'demoapp'

#ifdef RAYLIB_EXPORTS
	#define RAYLIB_API __declspec(dllexport) 
#else
	#define RAYLIB_API __declspec(dllimport) 
#endif

extern "C" RAYLIB_API int Raylib_Initialize();
extern "C" RAYLIB_API int Raylib_Terminate();
