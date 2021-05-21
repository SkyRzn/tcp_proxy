cmake_minimum_required(VERSION 3.0)

project (tcp_proxy C)

set(VERSION_INFO_MAJOR_VERSION 0)
set(VERSION_INFO_MINOR_VERSION 1)
set(VERSION_INFO_PATCH_VERSION 1)

add_definitions(-DVERSION_MAJOR=${VERSION_INFO_MAJOR_VERSION})
add_definitions(-DVERSION_MINOR=${VERSION_INFO_MINOR_VERSION})
add_definitions(-DVERSION_PATCH=${VERSION_INFO_PATCH_VERSION})
add_definitions(-DPROJECT_NAME=${PROJECT_NAME})

add_compile_options(
	"-Wall"
	"$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
	"$<$<CONFIG:RELEASE>:-O3>"
)

file(GLOB SOURCES "src/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries (${PROJECT_NAME} -luring)


