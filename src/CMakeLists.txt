# CMakeList.txt : CMake project for RayTracing, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

# Add source to this project's executable.
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/raylib)

# TODO: Why do I need them here
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/tinyobjloader/source)

file(GLOB_RECURSE SRCFILES "*.cc")
add_executable(RayTracer ${SRCFILES})

# TODO: Add tests and install targets if needed.
target_link_libraries(RayTracer raylib)

# TODO: These thirdparty libs are not built here, but their DLLs are required.
# Somehow let raylib's CMakeLists.txt copy them to this project's target dir?
add_custom_command(TARGET RayTracer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/binaries/FreeImage.dll"
	$<TARGET_FILE_DIR:RayTracer>)
add_custom_command(TARGET RayTracer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/OpenImageDenoise/bin/OpenImageDenoise.dll"
	$<TARGET_FILE_DIR:RayTracer>)
add_custom_command(TARGET RayTracer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/OpenImageDenoise/bin/tbb12.dll"
	$<TARGET_FILE_DIR:RayTracer>)

# TODO: Somehow copy raylib.dll to demo app, using install()?
add_custom_command(TARGET RayTracer POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${PROJECT_BINARY_DIR}/raylib/raylib.dll"
	$<TARGET_FILE_DIR:RayTracer>)

# TODO: This injects a hard-coded path to the executable.
#       Moving the project to another directory will make
#       the resource finder to fail.
add_compile_definitions(SOLUTION_DIR="${CMAKE_SOURCE_DIR}/")
