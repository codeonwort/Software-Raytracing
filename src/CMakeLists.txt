# CMakeList.txt : CMake project for CMakeProject1, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
include_directories(${PROJECT_SOURCE_DIR})
file(GLOB_RECURSE SRCFILES "*.cc" "*.cpp")
add_executable(RayTracer ${SRCFILES})

# TODO: Add tests and install targets if needed.
