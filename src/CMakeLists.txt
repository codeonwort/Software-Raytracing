# CMakeList.txt : CMake project for RayTracing, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/source)

link_directories(${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/binaries)

file(GLOB_RECURSE SRCFILES "*.cc" "*.cpp")
add_executable(RayTracer ${SRCFILES})

# TODO: Add tests and install targets if needed.
target_link_libraries(RayTracer FreeImage)

add_custom_command(TARGET RayTracer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/binaries/FreeImage.dll"
	$<TARGET_FILE_DIR:RayTracer>)