cmake_minimum_required(VERSION 3.10)

project(FileSharing)
set (SRC "./src")
set (INCLUDE "./include")

include_directories(${INCLUDE})

if(WIN32)
	add_executable(file_sharing ${SRC}/main.cpp)
	target_link_libraries(file_sharing gdi32 kernel32 user32)
	set (CMAKE_CXX_FLAGS "-std=c++20")
endif (WIN32)

if (UNIX)
    add_executable(file_sharing ${SRC}/main.cpp)
	target_link_libraries(file_sharing pthread dl  m)
	set (CMAKE_CXX_FLAGS "-std=c++20 -fsanitize=address")
	add_compile_definitions(_GLFW_X11)
endif (UNIX)
