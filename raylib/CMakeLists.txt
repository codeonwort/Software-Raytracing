# CMake project for raytracing module, built as DLL.
# Demo app projects will load this module.

# Reference: https://github.com/frymire/cmake-dll-demo

cmake_minimum_required (VERSION 3.8)

# https://learn.microsoft.com/en-us/cpp/build/reference/eh-exception-handling-model?view=msvc-170
# I dunno was it automatically added by Visual Studio?
# TODO: Determine proper flags for raylib and demo apps.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

# Add source to this project's executable.
include_directories(${PROJECT_SOURCE_DIR}/raylib)
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/source)
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/OpenImageDenoise/include)
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/tinyobjloader/source)

# Link libraries
link_directories(${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/binaries)
link_directories(${CMAKE_SOURCE_DIR}/thirdparty/OpenImageDenoise/lib)

file(GLOB_RECURSE RAYLIB_SOURCE_FILES "*.cc")

# Build as DLL
add_compile_definitions(RAYLIB_EXPORTS=1)
add_library(raylib SHARED ${RAYLIB_SOURCE_FILES})

# TODO: Is this an equivalent of
#     pragma comment(lib, "FreeImage.lib")
target_link_libraries(raylib FreeImage)

# Copy third-party DLLs after build
add_custom_command(TARGET raylib POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/FreeImage/binaries/FreeImage.dll"
	$<TARGET_FILE_DIR:raylib>)
add_custom_command(TARGET raylib POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/OpenImageDenoise/bin/OpenImageDenoise.dll"
	$<TARGET_FILE_DIR:raylib>)
add_custom_command(TARGET raylib POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_SOURCE_DIR}/thirdparty/OpenImageDenoise/bin/tbb12.dll"
	$<TARGET_FILE_DIR:raylib>)
