# CMake project for raytracing module, built as DLL.
# Demo app projects will load this module.

# Reference: https://github.com/frymire/cmake-dll-demo

# === TODO list === 
#
# [DllMain]
# https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
# How it would work with C# interop?

cmake_minimum_required (VERSION 3.8)

# https://learn.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-170
# I dunno was it automatically added by Visual Studio?
# TODO: Determine proper flags for raylib and demo apps.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

include_directories(${PROJECT_SOURCE_DIR}/raylib)

file(GLOB_RECURSE RAYLIB_SOURCE_FILES "*.cc")

# Build as DLL
add_compile_definitions(RAYLIB_EXPORTS=1)
add_library(raylib SHARED ${RAYLIB_SOURCE_FILES})
